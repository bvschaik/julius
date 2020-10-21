#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
    {TR_NO_PATCH_TITLE, "Патч 1.0.1.0 не установлен"},
    {TR_NO_PATCH_MESSAGE,
        "Установленная версия игры не содержит патча 1.0.1.0."
        "Вы можете скачать патч по следующему адресу:\n"
        URL_PATCHES "\n"
        "Продолжение работы без установки патча может вызвать сбои и ошибки!"},
    {TR_MISSING_FONTS_TITLE, "Шрифты не найдены"},
    {TR_MISSING_FONTS_MESSAGE,
        "Для данной версии игры требуются дополнительные файлы шрифтов."
        "Вы можете скачать файлы шрифтов для своей локализации по адресу:\n"
        URL_PATCHES},
    {TR_NO_EDITOR_TITLE, "Редактор не установлен"},
    {TR_NO_EDITOR_MESSAGE,
        "Установленная версия игры не содержит файлов редактора карт."
        "Вы можете скачать файлы редактора по адресу:\n"
        URL_EDITOR},
    {TR_INVALID_LANGUAGE_TITLE, "Ошибка пути к локализации"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "Выбранная папка не содержит необходимых файлов локализации."
        "Для просмотра ошибок обратитесь к  log-файлу."},
    {TR_BUTTON_OK, "Да"},
    {TR_BUTTON_CANCEL, "Отмена"},
    {TR_BUTTON_RESET_DEFAULTS, "Все по умолчанию"},
    {TR_BUTTON_CONFIGURE_HOTKEYS, "Горячие клавиши "},
    {TR_CONFIG_TITLE, "Настройки Julius"},
    {TR_CONFIG_LANGUAGE_LABEL, "Язык:"},
    {TR_CONFIG_LANGUAGE_DEFAULT, "(по умолчанию)"},
    {TR_CONFIG_HEADER_UI_CHANGES, "Настройки интерфейса"},
    {TR_CONFIG_HEADER_GAMEPLAY_CHANGES, "Настройки геймплея"},
    {TR_CONFIG_SHOW_INTRO_VIDEO, "Воспроизводить вступительные видео"},
    {TR_CONFIG_SIDEBAR_INFO, "Дополнительная информация на контрольной панели"},
    {TR_CONFIG_SMOOTH_SCROLLING, "Включить плавный скроллинг"},
    {TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, "Улучшить граф. эффект при очистке территории"},
    {TR_CONFIG_ALLOW_CYCLING_TEMPLES, "Постройка храмов в циклической последовательности"},
    {TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, "Зона покрытия при постройке резервуаров, колодцев и фонтанов"},
    {TR_CONFIG_SHOW_CONSTRUCTION_SIZE, "Показывать область при групповой стройке зданий"},
    {TR_CONFIG_HIGHLIGHT_LEGIONS, "Выделите легион при наведении курсора"}, // TODO: Google Translate
    {TR_CONFIG_SHOW_MILITARY_SIDEBAR, "Отображать боковую панель армии"},
    {TR_CONFIG_FIX_IMMIGRATION_BUG, "Исправить баг иммиграции в режиме \"Очень сложный\""},
    {TR_CONFIG_FIX_100_YEAR_GHOSTS, "Исправить баг 100-летних жителей"},
    {TR_CONFIG_FIX_EDITOR_EVENTS, "Исправить баг Императора и времени выживания в режиме конструктора"},
    {TR_HOTKEY_TITLE, "Горячие клавиши Julius"},
    {TR_HOTKEY_LABEL, "Горячие клавиши"},
    {TR_HOTKEY_ALTERNATIVE_LABEL, "Альтернативные"},
    {TR_HOTKEY_HEADER_ARROWS, "Клавиши стрелок"},
    {TR_HOTKEY_HEADER_GLOBAL, "Общие"},
    {TR_HOTKEY_HEADER_CITY, "В режиме города"},
    {TR_HOTKEY_HEADER_ADVISORS, "Советники"},
    {TR_HOTKEY_HEADER_OVERLAYS, "Слои"},
    {TR_HOTKEY_HEADER_BOOKMARKS, "Точки на карте города"},
    {TR_HOTKEY_HEADER_EDITOR, "Редактор карт"},
    {TR_HOTKEY_HEADER_BUILD, "Строительство"}, // TODO: Google Translate
    {TR_HOTKEY_ARROW_UP, "Вверх"},
    {TR_HOTKEY_ARROW_DOWN, "Вниз"},
    {TR_HOTKEY_ARROW_LEFT, "Влево"},
    {TR_HOTKEY_ARROW_RIGHT, "Вправо"},
    {TR_HOTKEY_TOGGLE_FULLSCREEN, "На весь экран/оконный режим"},
    {TR_HOTKEY_CENTER_WINDOW, "Окно по центру"},
    {TR_HOTKEY_RESIZE_TO_640, "Изменить размер на 640x480"},
    {TR_HOTKEY_RESIZE_TO_800, "Изменить размер на 800x600"},
    {TR_HOTKEY_RESIZE_TO_1024, "Изменить размер на 1024x768"},
    {TR_HOTKEY_SAVE_SCREENSHOT, "Скриншот видимого"},
    {TR_HOTKEY_SAVE_CITY_SCREENSHOT, "Скриншот всего города"},
    {TR_HOTKEY_BUILD_CLONE, "Клонировать здание под курсором"}, // TODO: Google Translate
    {TR_HOTKEY_LOAD_FILE, "Загрузить"},
    {TR_HOTKEY_SAVE_FILE, "Сохранить"},
    {TR_HOTKEY_INCREASE_GAME_SPEED, "Увеличить скорость игры"},
    {TR_HOTKEY_DECREASE_GAME_SPEED, "Уменьшить скорость игры"},
    {TR_HOTKEY_TOGGLE_PAUSE, "Пауза/продолжить"},
    {TR_HOTKEY_CYCLE_LEGION, "Перейти к след. легиону"},
    {TR_HOTKEY_ROTATE_MAP_LEFT, "Поворот карты влево"},
    {TR_HOTKEY_ROTATE_MAP_RIGHT, "Повернуть карту вправо"},
    {TR_HOTKEY_SHOW_ADVISOR_LABOR, "Труд"},
    {TR_HOTKEY_SHOW_ADVISOR_MILITARY, "Оборона"},
    {TR_HOTKEY_SHOW_ADVISOR_IMPERIAL, "Империя"},
    {TR_HOTKEY_SHOW_ADVISOR_RATINGS, "Рейтинги"},
    {TR_HOTKEY_SHOW_ADVISOR_TRADE, "Торговля"},
    {TR_HOTKEY_SHOW_ADVISOR_POPULATION, "Население"},
    {TR_HOTKEY_SHOW_ADVISOR_HEALTH, "Здоровье"},
    {TR_HOTKEY_SHOW_ADVISOR_EDUCATION, "Образование"},
    {TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, "Развлечения"},
    {TR_HOTKEY_SHOW_ADVISOR_RELIGION, "Религия"},
    {TR_HOTKEY_SHOW_ADVISOR_FINANCIAL, "Финансы"},
    {TR_HOTKEY_SHOW_ADVISOR_CHIEF, "Главный советник"},
    {TR_HOTKEY_TOGGLE_OVERLAY, "Вкл./выкл. слой"},
    {TR_HOTKEY_SHOW_OVERLAY_WATER, "Слой водоснабжения"},
    {TR_HOTKEY_SHOW_OVERLAY_FIRE, "Слой опасности пожара"},
    {TR_HOTKEY_SHOW_OVERLAY_DAMAGE, "Слой опасности разрушения"},
    {TR_HOTKEY_SHOW_OVERLAY_CRIME, "Слой опасности преступлений"},
    {TR_HOTKEY_SHOW_OVERLAY_PROBLEMS, "Слой проблем"},
    {TR_HOTKEY_GO_TO_BOOKMARK_1, "Перейти к точке 1"},
    {TR_HOTKEY_GO_TO_BOOKMARK_2, "Перейти к точке 2"},
    {TR_HOTKEY_GO_TO_BOOKMARK_3, "Перейти к точке 3"},
    {TR_HOTKEY_GO_TO_BOOKMARK_4, "Перейти к точке 4"},
    {TR_HOTKEY_SET_BOOKMARK_1, "Установить точку 1"},
    {TR_HOTKEY_SET_BOOKMARK_2, "Установить точку 2"},
    {TR_HOTKEY_SET_BOOKMARK_3, "Установить точку 3"},
    {TR_HOTKEY_SET_BOOKMARK_4, "Установить точку 4"},
    {TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, "Вкл./выкл. инфо о битвах"},
    {TR_HOTKEY_EDIT_TITLE, "Новая клавиша"}
};

void translation_russian(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
