#include "i18n.h"

static Language g_language = Language::EN;

static const char* const STRINGS[][3] = {
  {"GAMES", "ИГРЫ", "ІГРИ"},
  {"FILES", "ФАЙЛЫ", "ФАЙЛИ"},
  {"SETTINGS", "НАСТРОЙКИ", "ОПЦІЇ"},
  {"POWER", "ПИТАНИЕ", "ЖИВЛЕННЯ"},
  {"ABOUT", "О СИСТЕМЕ", "ПРО СИСТЕМУ"},
  {"ON", "ВКЛ", "УВІМК"},
  {"OFF", "ВЫКЛ", "ВИМК"},
  {"BACK", "НАЗАД", "НАЗАД"},
  {"OK", "ОК", "ОК"},
  {"YES", "ДА", "ТАК"},
  {"NO", "НЕТ", "НІ"},
  {"CANCEL", "ОТМЕНА", "СКАСУВАТИ"},
  {"RESET", "СБРОС", "СКИНУТИ"},
  {"SAVE", "СОХР.", "ЗБЕРЕГТИ"},
  {"LOAD", "ЗАГР.", "ЗАВАНТ."},
  {"NEW GAME", "ЗАНОВО", "НОВА ГРА"},
  {"CONTINUE", "ПРОДОЛЖИТЬ", "ДАЛІ"},
  {"EXIT", "ВЫХОД", "ВИХІД"},
  {"LANGUAGE", "ЯЗЫК", "МОВА"},
  {"WI-FI", "WI-FI", "WI-FI"},
  {"BLUETOOTH", "BLUETOOTH", "BLUETOOTH"},
  {"SOUND", "ЗВУК", "ЗВУК"},
  {"BOOT LOGO", "ЗАСТАВКА", "ЗАСТАВКА"},
  {"FACTORY RESET", "СБРОС", "СКИДАННЯ"},
  {"CLASSIC", "КЛАССИКА", "КЛАСИКА"},
  {"TEXT", "ТЕКСТ", "ТЕКСТ"},
  {"ENGLISH", "АНГЛИЙСКИЙ", "АНГЛІЙСЬКА"},
  {"РУССКИЙ", "РУССКИЙ", "РОСІЙСЬКА"},
  {"УКРАЇНСЬКА", "УКРАЇНСЬКА", "УКРАЇНСЬКА"},
  {"SETTINGS AND SAVES", "НАСТРОЙКИ И ИГРЫ", "НАЛАШТУВАННЯ Й ІГРИ"},
  {"WILL BE ERASED", "БУДУТ УДАЛЕНЫ", "БУДУТЬ ВИДАЛЕНІ"},
  {"FILES ARE KEPT", "ФАЙЛЫ ОСТАНУТСЯ", "ФАЙЛИ ЗАЛИШАТЬСЯ"},
  {"OPEN FILES", "ФАЙЛЫ", "ФАЙЛИ"},
  {"WI-FI UPLOAD", "ПО WI-FI", "ПО WI-FI"},
  {"BT UPLOAD", "ПО BLUETOOTH", "ПО BLUETOOTH"},
  {"UPLOAD", "ЗАГРУЗКА", "ЗАВАНТАЖ."},
  {"NO FILES", "НЕТ ФАЙЛОВ", "НЕМАЄ ФАЙЛІВ"},
  {"EMPTY FILE", "ПУСТОЙ ФАЙЛ", "ПОРОЖНІЙ ФАЙЛ"},
  {"FILE TOO BIG", "ФАЙЛ ВЕЛИК", "ФАЙЛ ЗАВЕЛИКИЙ"},
  {"OPEN FAILED", "ОШИБКА", "ПОМИЛКА"},
  {"WI-FI IS OFF", "WI-FI ВЫКЛ", "WI-FI ВИМК"},
  {"STARTING...", "ЗАПУСК...", "ЗАПУСК..."},
  {"AP START FAILED", "ОШИБКА WI-FI", "ПОМИЛКА WI-FI"},
  {"SSID: Mad_ESP", "SSID: Mad_ESP", "SSID: Mad_ESP"},
  {"PASS: mad_esp_1", "PASS: mad_esp_1", "PASS: mad_esp_1"},
  {"WEB: 192.168.4.1", "WEB: 192.168.4.1", "WEB: 192.168.4.1"},
  {"CLIENTS:", "КЛИЕНТЫ:", "КЛІЄНТИ:"},
  {"BT IS OFF", "BT ВЫКЛ", "BT ВИМК"},
  {"BLE START FAILED", "ОШИБКА BT", "ПОМИЛКА BT"},
  {"BLE: Mad_ESP", "BLE: Mad_ESP", "BLE: Mad_ESP"},
  {"MADPUT name.txt", "MADPUT name.txt", "MADPUT name.txt"},
  {"TEXT THEN MADEND", "ТЕКСТ, ПОТОМ MADEND", "ТЕКСТ, ПОТІМ MADEND"},
  {"PHONE: CONNECTED", "ТЕЛЕФОН: ЕСТЬ", "ТЕЛЕФОН: Є"},
  {"PHONE: WAITING", "ЖДУ ТЕЛЕФОН", "ЧЕКАЮ ТЕЛЕФОН"},
  {"RECEIVING:", "ПРИЕМ:", "ПРИЙОМ:"},
  {"CLICKER", "КЛИКЕР", "КЛІКЕР"},
  {"RUNNER", "БЕГУН", "БІГУН"},
  {"PONG", "ПОНГ", "ПОНГ"},
  {"HACK", "ВЗЛОМ", "ЗЛАМ"},
  {"SHOP", "МАГАЗИН", "КРАМНИЦЯ"},
  {"AUTO", "АВТО", "АВТО"},
  {"BEST", "РЕКОРД", "РЕКОРД"},
  {"SCORE", "СЧЕТ", "РАХУНОК"},
  {"CLICK+", "КЛИК+", "КЛІК+"},
  {"AUTO+", "АВТО+", "АВТО+"},
  {"BUY", "КУПИТЬ", "КУПИТИ"},
  {"MAX", "МАКС", "МАКС"},
  {"NO BITS", "НЕ ХВАТАЕТ", "НЕ ВИСТАЧАЄ"},
  {"PAUSE", "ПАУЗА", "ПАУЗА"},
  {"DIFFICULTY", "СЛОЖНОСТЬ", "СКЛАДНІСТЬ"},
  {"EASY", "ЛЕГКО", "ЛЕГКО"},
  {"NORMAL", "НОРМА", "НОРМА"},
  {"HARD", "СЛОЖНО", "СКЛАДНО"},
  {"SAVED", "СОХРАНЕНО", "ЗБЕРЕЖЕНО"},
  {"NO SAVE", "НЕТ СОХР.", "НЕМАЄ ЗБЕРЕЖ."},
  {"GAME OVER", "КОНЕЦ ИГРЫ", "КІНЕЦЬ ГРИ"},
  {"NEW RECORD", "НОВЫЙ РЕКОРД", "НОВИЙ РЕКОРД"},
  {"LEVEL CLEAR", "УРОВЕНЬ ПРОЙДЕН", "РІВЕНЬ ПРОЙДЕНО"},
  {"YOU", "ТЫ", "ТИ"},
  {"CPU", "ЦП", "ЦП"},
  {"R1 SERVE", "R1 ПОДАЧА", "R1 ПОДАЧА"},
  {"YOU WIN", "ТЫ ПОБЕДИЛ", "ТИ ПЕРЕМІГ"},
  {"YOU LOSE", "ТЫ ПРОИГРАЛ", "ТИ ПРОГРАВ"},
  {"SLEEP MODE", "СПЯЩИЙ РЕЖИМ", "СПЛЯЧИЙ РЕЖИМ"},
  {"RESTART", "ПЕРЕЗАПУСК", "ПЕРЕЗАВАНТ."},
  {"POWER OFF", "ВЫКЛЮЧИТЬ", "ВИМКНУТИ"},
  {"SLEEPING...", "ЗАСЫПАЮ...", "ЗАСИНАЮ..."},
  {"REBOOT...", "РЕБУТ...", "РЕБУТ..."},
  {"SCREEN OFF", "ЭКРАН ВЫКЛ.", "ЕКРАН ВИКЛ."},
  {"PRESS ANY KEY", "НАЖМИ ЛЮБУЮ", "НАТИСНИ БУДЬ-ЯКУ"},
  {"LOW BATTERY", "НИЗКИЙ ЗАРЯД", "НИЗЬКИЙ ЗАРЯД"},
  {"CHARGING", "ЗАРЯДКА", "ЗАРЯДЖАННЯ"},
  {"VERSION", "ВЕРСИЯ", "ВЕРСІЯ"},
  {"DEVICE", "УСТРОЙСТВО", "ПРИСТРІЙ"},
  {"BATTERY", "БАТАРЕЯ", "БАТАРЕЯ"},
  {"TEMP", "ТЕМП", "ТЕМП"},
  {"RAM", "RAM", "RAM"},
  {"UP", "ВРЕМЯ", "ЧАС"},
  {"HEAP", "ПАМЯТЬ", "ПАМ'ЯТЬ"},
  {"FLASH", "ФЛЕШ", "ФЛЕШ"},
  {"MAC", "MAC", "MAC"},
  {"IP", "IP", "IP"},
  {"FONT", "ШРИФТ", "ШРИФТ"},
  {"PAGE", "СТР.", "СТОР."},
  {"R1 START", "R1 СТАРТ", "R1 СТАРТ"},
  {"R2 BACK", "R2 НАЗАД", "R2 НАЗАД"},
  {"R1 OK", "R1 ОК", "R1 ОК"},
  {"R1 CONFIRM", "R1 ПОДТВЕРДИТЬ", "R1 ПІДТВЕРДИТИ"},
  {"R1 WAKE", "R1 ПРОБУЖДЕНИЕ", "R1 ПРОБУДЖЕННЯ"},
  {"FACTORY RESET?", "СБРОС?", "СКИНУТИ?"},
  {"NOT A TEXT FILE", "НЕ ТЕКСТОВЫЙ", "НЕ ТЕКСТОВИЙ"},
  {"TURN ON", "ВКЛЮЧИТЬ", "УВІМКНУТИ"},
  {"LEVEL", "УРОВЕНЬ", "РІВЕНЬ"},
  {"BARK", "ГАВ", "ГАВ"},
  {"PAUSED", "ПАУЗА", "ПАУЗА"},
  {"NEW", "НОВАЯ", "НОВА"},
  {"NO SAVE DATA", "НЕТ СОХРАНЕНИЯ", "НЕМАЄ ЗБЕРЕЖЕННЯ"},
  {"SAVE GAME", "СОХРАНИТЬ", "ЗБЕРЕГТИ ГРУ"},
  {"LOAD GAME", "ЗАГРУЗИТЬ", "ЗАВАНТАЖИТИ"},
  {"GAME SETTINGS", "НАСТРОЙКИ ИГРЫ", "НАЛАШТУВАННЯ ГРИ"},
  {"EXIT GAME", "ВЫЙТИ ИЗ ИГРЫ", "ВИЙТИ З ГРИ"},
  {"CONTINUE", "ПРОДОЛЖИТЬ", "ПРОДОЛЖИТИ"},
  {"LIVES", "ЖИЗНИ", "ЖИТТЯ"},
  {"BITS", "БИТЫ", "БІТИ"},
  {"DIST", "ДИСТ.", "ДИСТ."},
  {"CPU", "ЦП", "ЦП"},
  {"UP", "ВР", "ЧАС"},

  {"RUN", "БЕГ", "БІГ"},
  {"JUMP", "ПРЫЖОК", "СТРИБОК"},

  {"LOADED", "ЗАГРУЖЕНО", "ЗАВАНТАЖЕНО"},
};

void setLanguage(Language lang) {
  if ((uint8_t)lang > 2) lang = Language::EN;
  g_language = lang;
}

Language currentLanguage() { return g_language; }

const char* tr(Str id) {
  uint16_t i = (uint16_t)id;
  if (i >= (uint16_t)Str::COUNT) return "?";
  return STRINGS[i][(uint8_t)g_language];
}

const char* languageName(Language lang) {
  switch (lang) {
    case Language::RU: return tr(Str::LANG_RUSSIAN);
    case Language::UK: return tr(Str::LANG_UKRAINIAN);
    default: return tr(Str::LANG_ENGLISH);
  }
}

const char* languageCode(Language lang) {
  switch (lang) {
    case Language::RU: return "RU";
    case Language::UK: return "UK";
    default: return "EN";
  }
}
