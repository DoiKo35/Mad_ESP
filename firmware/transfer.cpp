#include "transfer.h"
#include "core.h"
#include "config.h"
#include "games_store.h"

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <algorithm>
#include <vector>

// ============================================================================
//  Shared filename sanitising
// ============================================================================
bool sanitizeFileName(const char* input, char* output, size_t outputSize) {
  if (!input || !output || outputSize < 5) return false;
  const char* base = input;
  for (const char* p = input; *p; ++p) {
    if (*p == '/' || *p == '\\') base = p + 1;
  }

  size_t written = 0;
  bool lastUnderscore = false;
  for (const unsigned char* p = (const unsigned char*)base; *p && written < outputSize - 1; ++p) {
    unsigned char c = *p;
    bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
    if (ok) {
      output[written++] = (char)c;
      lastUnderscore = (c == '_');
    } else if (c == ' ') {
      if (!lastUnderscore && written < outputSize - 1) {
        output[written++] = '_';
        lastUnderscore = true;
      }
    }
  }
  output[written] = 0;

  while (written > 0 && output[written - 1] == '.') output[--written] = 0;
  while (written > 0 && output[0] == '.') {
    memmove(output, output + 1, written);
    --written;
  }
  if (written == 0) return false;

  if (!strchr(output, '.')) {
    if (written + 4 >= outputSize) written = (written > outputSize - 5) ? outputSize - 5 : written;
    output[written] = 0;
    strncat(output, ".txt", outputSize - strlen(output) - 1);
  }
  output[outputSize - 1] = 0;
  return strlen(output) > 0;
}

// ============================================================================
//  Wi-Fi AP + web file bay
// ============================================================================
static WebServer webServer(80);
static bool g_routesReady = false;
static bool g_webStarted = false;

static File g_uploadFile;
static bool g_uploadOk = false;
static char g_uploadName[MAX_FILENAME_LENGTH + 1] = {0};
static size_t g_uploadWritten = 0;

static void abortWebUpload() {
  if (g_uploadFile) g_uploadFile.close();
  if (g_uploadName[0]) {
    char path[48];
    snprintf(path, sizeof(path), "/%s", g_uploadName);
    LittleFS.remove(path);
    g_uploadName[0] = 0;
  }
  g_uploadOk = false;
  g_uploadWritten = 0;
}

static String htmlEscape(const String& value) {
  String out;
  out.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    char c = value[i];
    switch (c) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      case '\'': out += F("&#39;"); break;
      default: out += c; break;
    }
  }
  return out;
}

static String humanSize(size_t bytes) {
  char buffer[24];
  if (bytes >= 1024UL * 1024UL) {
    snprintf(buffer, sizeof(buffer), "%.1f MB", bytes / (1024.0 * 1024.0));
  } else if (bytes >= 1024UL) {
    snprintf(buffer, sizeof(buffer), "%.1f KB", bytes / 1024.0);
  } else {
    snprintf(buffer, sizeof(buffer), "%u B", (unsigned)bytes);
  }
  return String(buffer);
}

static void handleWebRoot() {
  std::vector<String> names;
  File root = LittleFS.open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        String name = file.name();
        while (name.startsWith("/")) name.remove(0, 1);
        names.push_back(name);
      }
      file = root.openNextFile();
    }
  }
  std::sort(names.begin(), names.end());

  String page;
  page.reserve(3200 + names.size() * 180);
  page += F("<!doctype html><html><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>Mad_ESP file bay</title><style>"
            ":root{color-scheme:dark}body{font-family:Cascadia Mono,Consolas,monospace;"
            "background:#080b0a;color:#c9ffdc;margin:0;padding:18px}"
            "h1{font-size:22px;letter-spacing:2px;margin:0 0 4px;color:#fff}"
            ".sub{color:#6b9b7a;font-size:12px;margin-bottom:18px}"
            "main{max-width:720px;margin:auto}section{border:1px solid #2a5a3a;"
            "padding:14px;margin:12px 0;background:#0c120f}"
            "a,button{color:#07110a;background:#8dffb0;border:0;padding:8px 11px;"
            "font:inherit;text-decoration:none;cursor:pointer}"
            "button.del{background:#26332a;color:#ffadad;padding:5px 8px}"
            "input[type=file]{width:100%;margin:10px 0;color:#d8ffe5}"
            "li{display:flex;justify-content:space-between;align-items:center;gap:12px;"
            "padding:8px 0;border-bottom:1px dotted #264832;list-style:none}"
            "ul{padding:0;margin:0}.name{overflow-wrap:anywhere}.meta{color:#6b9b7a;"
            "font-size:12px;white-space:nowrap}.msg{padding:9px;background:#173b24;"
            "border-left:3px solid #8dffb0}footer{color:#456b52;font-size:11px;"
            "margin-top:18px}</style></head><body><main>");
  page += F("<h1>Mad_ESP // FILE BAY</h1><div class='sub'>Wi-Fi: ");
  page += WIFI_AP_SSID;
  page += F(" &middot; ");
  page += WiFi.softAPIP().toString();
  page += F("</div>");
  if (webServer.hasArg("ok")) page += F("<div class='msg'>File uploaded.</div>");
  page += F("<section><form method='post' action='/upload' enctype='multipart/form-data'>"
            "<h3>UPLOAD FILE</h3><input type='file' name='file' required>"
            "<div><button type='submit'>SEND TO Mad_ESP</button></div>"
            "</form></section><section><h3>FILES</h3><ul>");

  if (names.empty()) {
    page += F("<li><span class='meta'>No files yet.</span></li>");
  } else {
    for (const String& name : names) {
      char path[48];
      snprintf(path, sizeof(path), "/%s", name.c_str());
      File file = LittleFS.open(path, "r");
      size_t size = file ? file.size() : 0;
      if (file) file.close();
      page += F("<li><div><div class='name'>");
      page += htmlEscape(name);
      page += F("</div><div class='meta'>");
      page += humanSize(size);
      page += F("</div></div><form method='post' action='/delete' onsubmit=\"return confirm('Delete this file?')\">"
                "<input type='hidden' name='name' value='");
      page += htmlEscape(name);
      page += F("'><button class='del' type='submit'>DEL</button></form></li>");
    }
  }

  page += F("</ul></section><section><div class='meta'>");
  page += F("Storage: ");
  page += humanSize(LittleFS.usedBytes());
  page += F(" / ");
  page += humanSize(LittleFS.totalBytes());
  page += F("</div></section><footer>Mad_ESP Alpha 1.0.0 &middot; uploads are stored in LittleFS</footer>"
            "</main></body></html>");
  webServer.send(200, "text/html; charset=utf-8", page);
}

static void handleUploadDone() {
  if (g_uploadOk) {
    webServer.sendHeader("Location", "/?ok=1");
    webServer.send(303, "text/plain", "Uploaded");
  } else {
    webServer.send(507, "text/plain", "Upload failed (name, storage or size limit)");
  }
  g_uploadOk = false;
}

static void handleUploadStream() {
  HTTPUpload& upload = webServer.upload();
  switch (upload.status) {
    case UPLOAD_FILE_START: {
      // A previous interrupted request may still hold a file handle.
      abortWebUpload();
      char safe[MAX_FILENAME_LENGTH + 1] = {0};
      if (!sanitizeFileName(upload.filename.c_str(), safe, sizeof(safe))) {
        return;
      }
      strlcpy(g_uploadName, safe, sizeof(g_uploadName));
      char path[48];
      snprintf(path, sizeof(path), "/%s", g_uploadName);
      if (LittleFS.totalBytes() - LittleFS.usedBytes() < 2048) return;
      g_uploadFile = LittleFS.open(path, "w");
      g_uploadOk = (bool)g_uploadFile;
      break;
    }
    case UPLOAD_FILE_WRITE: {
      if (!g_uploadOk) return;
      if (g_uploadWritten + upload.currentSize > MAX_UPLOAD_FILE_BYTES) {
        g_uploadOk = false;
        break;
      }
      size_t written = g_uploadFile.write(upload.buf, upload.currentSize);
      g_uploadWritten += written;
      if (written != upload.currentSize) g_uploadOk = false;
      break;
    }
    case UPLOAD_FILE_END: {
      if (g_uploadFile) g_uploadFile.close();
      if (!g_uploadOk) {
        abortWebUpload();
      } else {
        g_uploadName[0] = 0;
        g_uploadWritten = 0;
        // Keep g_uploadOk set until handleUploadDone() responds. The next
        // UPLOAD_FILE_START resets it through abortWebUpload().
      }
      break;
    }
    case UPLOAD_FILE_ABORTED: {
      abortWebUpload();
      break;
    }
    default:
      break;
  }
}

static void handleDelete() {
  char safe[MAX_FILENAME_LENGTH + 1] = {0};
  if (webServer.hasArg("name") &&
      sanitizeFileName(webServer.arg("name").c_str(), safe, sizeof(safe))) {
    char path[48];
    snprintf(path, sizeof(path), "/%s", safe);
    LittleFS.remove(path);
  }
  webServer.sendHeader("Location", "/");
  webServer.send(303, "text/plain", "Deleted");
}

static void setupWebRoutes() {
  if (g_routesReady) return;
  webServer.on("/", HTTP_GET, handleWebRoot);
  webServer.on("/upload", HTTP_POST, handleUploadDone, handleUploadStream);
  webServer.on("/delete", HTTP_POST, handleDelete);
  webServer.on("/favicon.ico", HTTP_GET, []() { webServer.send(204, "text/plain", ""); });
  webServer.onNotFound([]() {
    webServer.send(404, "text/plain", "Not found. Open http://192.168.4.1/");
  });
  g_routesReady = true;
}

bool WifiUploadService::begin() {
  if (_active) return true;
  _lastError = "";
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL,
                        false, WIFI_AP_CLIENTS);
  if (!ok) {
    WiFi.mode(WIFI_OFF);
    _lastError = "AP start failed";
    return false;
  }
  setupWebRoutes();
  webServer.begin();
  g_webStarted = true;
  _active = true;
  return true;
}

void WifiUploadService::end() {
  if (!_active) return;
  abortWebUpload();
  webServer.stop();
  g_webStarted = false;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  _active = false;
}

void WifiUploadService::tick() {
  if (_active) webServer.handleClient();
}

uint8_t WifiUploadService::clients() const {
  return _active ? WiFi.softAPgetStationNum() : 0;
}

// ============================================================================
//  BLE UART file upload (Nordic UART Service)
// ============================================================================
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEAdvertising.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_gap_ble_api.h>

static bool g_bleInitialized = false;
static bool g_bleAllowAdvertising = false;
static bool g_bleClientConnected = false;
static uint16_t g_bleConnId = 0;
static BLEServer* g_bleServer = nullptr;
static BLECharacteristic* g_bleTx = nullptr;
static QueueHandle_t g_bleQueue = nullptr;
static String g_bleLine;
static bool g_bleLineOverflow = false;
static File g_bleFile;
static uint32_t g_bleBytes = 0;
static char g_bleFileName[MAX_FILENAME_LENGTH + 1] = {0};

static void bleNotify(const char* text) {
  if (!g_bleTx || !g_bleClientConnected || !text) return;
  g_bleTx->setValue(std::string(text));
  g_bleTx->notify();
}

static void bleNotify(const String& text) {
  bleNotify(text.c_str());
}

static void finishBleUpload(bool success);

class BleServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    g_bleClientConnected = true;
    g_bleConnId = server->getConnId();
    bleNotify("Mad_ESP ready. MADHELP for commands");
  }
  void onDisconnect(BLEServer* server) override {
    (void)server;
    g_bleClientConnected = false;
    g_bleConnId = 0;
    // Drop a partially received file and release its LittleFS handle.
    if (g_bleFileName[0]) finishBleUpload(false);
    if (g_bleAllowAdvertising) {
      BLEDevice::getAdvertising()->start();
    }
  }
};

static void queueBleLine(const char* data, size_t length) {
  if (!g_bleQueue || length == 0) return;
  char* copy = (char*)malloc(length + 1);
  if (!copy) return;
  memcpy(copy, data, length);
  copy[length] = 0;
  if (xQueueSend(g_bleQueue, &copy, 0) != pdTRUE) free(copy);
}

class BleRxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    std::string value = characteristic->getValue();
    for (size_t i = 0; i < value.size(); ++i) {
      char c = value[i];
      if (c == '\r' || c == '\n') {
        if (g_bleLine.length() > 0) {
          if (!g_bleLineOverflow) queueBleLine(g_bleLine.c_str(), g_bleLine.length());
          g_bleLine = "";
        }
        g_bleLineOverflow = false;
      } else if (g_bleLine.length() < BLE_LINE_LIMIT) {
        g_bleLine += c;
      } else {
        g_bleLineOverflow = true;
        g_bleLine = "";
      }
    }
  }
};

static void finishBleUpload(bool success) {
  if (g_bleFile) g_bleFile.close();
  char path[48];
  snprintf(path, sizeof(path), "/%s", g_bleFileName);
  if (!success && g_bleFileName[0]) LittleFS.remove(path);
  g_bleFileName[0] = 0;
  g_bleBytes = 0;
}

static bool beginBleUpload(const char* requestedName) {
  char safe[MAX_FILENAME_LENGTH + 1] = {0};
  if (!sanitizeFileName(requestedName, safe, sizeof(safe))) {
    bleNotify("ERR bad filename");
    return false;
  }
  if (g_bleFile) g_bleFile.close();
  char path[48];
  snprintf(path, sizeof(path), "/%s", safe);
  g_bleFile = LittleFS.open(path, "w");
  if (!g_bleFile) {
    bleNotify("ERR open file");
    return false;
  }
  strlcpy(g_bleFileName, safe, sizeof(g_bleFileName));
  g_bleBytes = 0;
  char reply[64];
  snprintf(reply, sizeof(reply), "READY %s", safe);
  bleNotify(reply);
  return true;
}

static void sendBleFileList() {
  File root = LittleFS.open("/");
  if (!root) { bleNotify("ERR fs"); return; }
  bleNotify("FILES:");
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();
      while (name.startsWith("/")) name.remove(0, 1);
      char line[80];
      snprintf(line, sizeof(line), "%s %lu", name.c_str(), (unsigned long)file.size());
      bleNotify(line);
    }
    file = root.openNextFile();
  }
  bleNotify("END LS");
}

static void handleBleCommand(const char* rawLine) {
  String line(rawLine);
  line.trim();
  if (line.length() == 0) return;

  if (line == "MADEND") {
    if (g_bleFileName[0]) {
      char savedName[MAX_FILENAME_LENGTH + 1];
      strlcpy(savedName, g_bleFileName, sizeof(savedName));
      uint32_t bytes = g_bleBytes;
      finishBleUpload(true);
      char reply[80];
      snprintf(reply, sizeof(reply), "SAVED %s %lu", savedName, (unsigned long)bytes);
      bleNotify(reply);
    } else {
      bleNotify("ERR no upload");
    }
    return;
  }

  if (g_bleFileName[0]) {
    if (g_bleBytes + line.length() + 1 > MAX_UPLOAD_FILE_BYTES) {
      char failedName[MAX_FILENAME_LENGTH + 1];
      strlcpy(failedName, g_bleFileName, sizeof(failedName));
      finishBleUpload(false);
      char reply[80];
      snprintf(reply, sizeof(reply), "ERR too big %s", failedName);
      bleNotify(reply);
      return;
    }
    size_t written = g_bleFile.write((const uint8_t*)line.c_str(), line.length());
    if (written == line.length()) {
      g_bleFile.write('\n');
      g_bleBytes += written + 1;
    } else {
      finishBleUpload(false);
      bleNotify("ERR write failed");
    }
    return;
  }

  if (line.startsWith("MADPUT ")) {
    if (!beginBleUpload(line.c_str() + 7)) return;
    return;
  }
  if (line.startsWith("MADDEL ")) {
    char safe[MAX_FILENAME_LENGTH + 1] = {0};
    if (!sanitizeFileName(line.c_str() + 7, safe, sizeof(safe))) {
      bleNotify("ERR bad filename");
      return;
    }
    char path[48];
    snprintf(path, sizeof(path), "/%s", safe);
    bool ok = LittleFS.remove(path);
    bleNotify(ok ? "DELETED" : "ERR no file");
    return;
  }
  if (line == "MADLS") { sendBleFileList(); return; }
  if (line == "MADHELP") {
    bleNotify("MADPUT name.txt");
    bleNotify("send text lines");
    bleNotify("MADEND = save");
    bleNotify("MADLS / MADDEL");
    return;
  }
  bleNotify("ERR command (MADHELP)");
}

bool BleUploadService::begin() {
  if (_active) return true;
  _lastError = "";
  if (!g_bleInitialized) {
    if (!g_bleQueue) g_bleQueue = xQueueCreate(16, sizeof(char*));
    if (!g_bleQueue) {
      _lastError = "No BLE queue";
      return false;
    }
    BLEDevice::init(BLE_DEVICE_NAME);
    g_bleServer = BLEDevice::createServer();
    g_bleServer->setCallbacks(new BleServerCallbacks());
    BLEService* service = g_bleServer->createService(BLE_NUS_SERVICE_UUID);

    BLECharacteristic* rx = service->createCharacteristic(
        BLE_NUS_RX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    rx->setCallbacks(new BleRxCallbacks());

    g_bleTx = service->createCharacteristic(BLE_NUS_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    g_bleTx->addDescriptor(new BLE2902());

    service->start();
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_NUS_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMinPreferred(0x12);
    g_bleInitialized = true;
  }

  g_bleAllowAdvertising = true;
  g_bleClientConnected = false;
  g_bleLine = "";
  g_bleLineOverflow = false;
  BLEDevice::getAdvertising()->start();
  _active = true;
  return true;
}

void BleUploadService::end() {
  if (!_active) return;
  _active = false;
  g_bleAllowAdvertising = false;
  g_bleLine = "";
  g_bleLineOverflow = false;
  BLEDevice::getAdvertising()->stop();
  if (g_bleClientConnected && g_bleServer) {
    g_bleServer->disconnect(g_bleConnId);
    g_bleClientConnected = false;
  }
  if (g_bleFileName[0]) finishBleUpload(false);
}

void BleUploadService::tick() {
  _connected = g_bleClientConnected;
  _receiving = g_bleFileName[0] != 0;
  _receivedBytes = g_bleBytes;
  if (g_bleFileName[0]) strlcpy(_fileName, g_bleFileName, sizeof(_fileName));
  else _fileName[0] = 0;

  if (!g_bleQueue) return;
  char* line = nullptr;
  for (uint8_t processed = 0; processed < 6; ++processed) {
    if (xQueueReceive(g_bleQueue, &line, 0) != pdTRUE) break;
    if (_active) {
      if (!_connected) {
        // A disconnect during transfer means the file is incomplete.
        if (g_bleFileName[0]) finishBleUpload(false);
      } else {
        handleBleCommand(line);
      }
    }
    free(line);
  }
}

