#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
    //{TR_NO_PATCH_TITLE, "Patch 1.0.1.0 not installed"},
    {TR_NO_PATCH_MESSAGE,
        "В вашей установке Caesar 3 нет установленного патча 1.0.1.0.\n"
        "Вы можете скачать патч с:\n"
        URL_PATCHES "\n"
        "Продолжайте на свой страх и риск."},
    /*{TR_MISSING_FONTS_TITLE, "Missing fonts"},
    {TR_MISSING_FONTS_MESSAGE,
        "Your Caesar 3 installation requires extra font files. "
        "You can download them for your language from:\n"
        "https://github.com/bvschaik/julius/wiki/Patches"},
    {TR_NO_EDITOR_TITLE, "Editor not installed"},*/
    {TR_NO_EDITOR_MESSAGE,
        "Ваша установка Caesar 3 не содержит файлов редактора.\n"
        "Вы можете скачать их с:\n"
        URL_EDITOR},/*
    {TR_INVALID_LANGUAGE_TITLE, "Invalid language directory"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "The directory you selected does not contain a valid language pack. "
        "Please check the log for errors."},
    {BUTTON_OK, "OK"},
    {BUTTON_CANCEL, "Cancel"},
    {BUTTON_RESET_DEFAULTS, "Reset defaults"},
    {BUTTON_CONFIGURE_HOTKEYS, "Configure hotkeys"},*/
};

void translation_russian(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
