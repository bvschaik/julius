#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
    {TR_NO_PATCH_TITLE, "Patch 1.0.1.0 not installed"},
    {TR_NO_PATCH_MESSAGE,
        "Your Caesar 3 installation does not have the 1.0.1.0 patch installed. "
        "You can download the patch from:\n"
        URL_PATCHES "\n"
        "Continue at your own risk."},
    {TR_MISSING_FONTS_TITLE, "Missing fonts"},
    {TR_MISSING_FONTS_MESSAGE,
        "Your Caesar 3 installation requires extra font files. "
        "You can download them for your language from:\n"
        URL_PATCHES},
    {TR_NO_EDITOR_TITLE, "Editor not installed"},
    {TR_NO_EDITOR_MESSAGE,
        "Your Caesar 3 installation does not contain the editor files. "
        "You can download them from:\n"
        URL_EDITOR},
    {TR_INVALID_LANGUAGE_TITLE, "Invalid language directory"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "The directory you selected does not contain a valid language pack. "
        "Please check the log for errors."},
    {TR_BUTTON_OK, "OK"},
    {TR_BUTTON_CANCEL, "Cancel"},
    {TR_BUTTON_RESET_DEFAULTS, "Reset defaults"},
    {TR_BUTTON_CONFIGURE_HOTKEYS, "Configure hotkeys"},
    {TR_CONFIG_TITLE, "Julius configuration options"},
    {TR_CONFIG_LANGUAGE_LABEL, "Language:"},
    {TR_CONFIG_LANGUAGE_DEFAULT, "(default)"},
    {TR_CONFIG_HEADER_UI_CHANGES, "User interface changes"},
    {TR_CONFIG_HEADER_GAMEPLAY_CHANGES, "Gameplay changes"},
    {TR_CONFIG_SHOW_INTRO_VIDEO, "Play intro videos"},
    {TR_CONFIG_SIDEBAR_INFO, "Extra information in the control panel"},
    {TR_CONFIG_SMOOTH_SCROLLING, "Enable smooth scrolling"},
    {TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, "Improve visual feedback when clearing land"},
    {TR_CONFIG_ALLOW_CYCLING_TEMPLES, "Allow building each temple in succession"},
    {TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, "Show range when building reservoirs, fountains and wells"},
    {TR_CONFIG_SHOW_CONSTRUCTION_SIZE, "Show draggable construction size"},
    {TR_CONFIG_FIX_IMMIGRATION_BUG, "Fix immigration bug on very hard"},
    {TR_CONFIG_FIX_100_YEAR_GHOSTS, "Fix 100-year-old ghosts"},
    {TR_CONFIG_FIX_EDITOR_EVENTS, "Fix Emperor change and survival time in custom missions"},
    {TR_HOTKEY_TITLE, "Julius hotkey configuration"},
    {TR_HOTKEY_LABEL, "Hotkey"},
    {TR_HOTKEY_ALTERNATIVE_LABEL, "Alternative"},
    {TR_HOTKEY_EDIT_TITLE, "Press new hotkey"}
};

void translation_english(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
