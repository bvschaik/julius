#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
    {TR_NO_PATCH_TITLE, "Uppdatering 1.0.1.0 inte installerad"},
    {TR_NO_PATCH_MESSAGE,
        "Din Caesar 3 installation har inte 1.0.1.0 uppdateringen installerad. "
        "Du kan ladda ner installationen från:\n"
        URL_PATCHES "\n"
        "Fortsätt på egen risk."},
    {TR_MISSING_FONTS_TITLE, "Saknade typsnitt"},
    {TR_MISSING_FONTS_MESSAGE,
        "Din Caesar 3 installation behöver extra typsnittsfiler. "
        "Du kan ladda ner dom för ditt språk från:\n"
    URL_PATCHES},
    {TR_NO_EDITOR_TITLE, "Redigeringsverktyg inte installerade"},
    {TR_NO_EDITOR_MESSAGE,
        "Din Caesar 3 installation saknar redigeringsfilerna. "
        "Du kan ladda ner dom från:\n"
    URL_EDITOR},
    {TR_INVALID_LANGUAGE_TITLE, "Ogiltig språkmapp"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "Den valda mappen inehåller inte ett giltigt språkpaket. "
        "Kolla loggen efter fel."},
    {TR_BUILD_ALL_TEMPLES, "Alla"},
    {TR_BUTTON_OK, "OK"},
    {TR_BUTTON_CANCEL, "Avbryt"},
    {TR_BUTTON_RESET_DEFAULTS, "Återställ förval"},
    {TR_BUTTON_CONFIGURE_HOTKEYS, "Kortkommando"},
    {TR_CONFIG_TITLE, "Julius konfigurationer"},
    {TR_CONFIG_LANGUAGE_LABEL, "Språk:"},
    {TR_CONFIG_LANGUAGE_DEFAULT, "(förval)"},
    {TR_CONFIG_DISPLAY_SCALE, "Bildskärmsskala:"},
    {TR_CONFIG_CURSOR_SCALE, "Markörskala:"},
    {TR_CONFIG_HEADER_UI_CHANGES, "Användargränssnittsändringar"},
    {TR_CONFIG_HEADER_GAMEPLAY_CHANGES, "Spelinställningar"},
    {TR_CONFIG_SHOW_INTRO_VIDEO, "Visa introduktionsfilmer"},
    {TR_CONFIG_SIDEBAR_INFO, "Extra information i kontrollpanelen"},
    {TR_CONFIG_SMOOTH_SCROLLING, "Mjuk scrollning"},
    {TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG, "Desativar rolagem do mapa com botão direito do mouse"},
    {TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, "Öka visuel återkoppling vid rensning av land"},
    {TR_CONFIG_ALLOW_CYCLING_TEMPLES, "Tillåt att bygga varje tempel i följd"},
    {TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, "Visa räckvidd vid bygge av reservoarer, fontäner och brunnar"},
    {TR_CONFIG_SHOW_CONSTRUCTION_SIZE, "Visa dragbar byggnadsstorlek"},
    {TR_CONFIG_HIGHLIGHT_LEGIONS, "Markera legion vid mushovring"},
    {TR_CONFIG_SHOW_MILITARY_SIDEBAR, "Visa militär sidopanel"},
    {TR_CONFIG_FIX_IMMIGRATION_BUG, "Fixa invandringsproblem vid väldigt svårt svårighetsgrad"},
    {TR_CONFIG_FIX_100_YEAR_GHOSTS, "Fixa 100-års spöken"},
    {TR_HOTKEY_TITLE, "Julius kortkommandon"},
    {TR_HOTKEY_LABEL, "Kortkommando"},
    {TR_HOTKEY_ALTERNATIVE_LABEL, "Alternativ"},
    {TR_HOTKEY_HEADER_ARROWS, "Piltangenter"},
    {TR_HOTKEY_HEADER_GLOBAL, "Globala kortkommandon"},
    {TR_HOTKEY_HEADER_CITY, "Stadskortkommandon"},
    {TR_HOTKEY_HEADER_ADVISORS, "Rådgivare"},
    {TR_HOTKEY_HEADER_OVERLAYS, "Överlägg"},
    {TR_HOTKEY_HEADER_BOOKMARKS, "Stadskarta bokmärken"},
    {TR_HOTKEY_HEADER_EDITOR, "Redigeringsverktyg"},
    {TR_HOTKEY_HEADER_BUILD, "Byggnadskortkommandon"},
    {TR_HOTKEY_ARROW_UP, "Upp"},
    {TR_HOTKEY_ARROW_DOWN, "Ner"},
    {TR_HOTKEY_ARROW_LEFT, "Vänster"},
    {TR_HOTKEY_ARROW_RIGHT, "Höger"},
    {TR_HOTKEY_TOGGLE_FULLSCREEN, "Fullskärm"},
    {TR_HOTKEY_CENTER_WINDOW, "Centrera fönster"},
    {TR_HOTKEY_RESIZE_TO_640, "Sätt fönster till 640x480"},
    {TR_HOTKEY_RESIZE_TO_800, "Sätt fönster till 800x600"},
    {TR_HOTKEY_RESIZE_TO_1024, "Sätt fönster till 1024x768"},
    {TR_HOTKEY_SAVE_SCREENSHOT, "Spara skärmdump"},
    {TR_HOTKEY_SAVE_CITY_SCREENSHOT, "Spara staden som skärmdump"},
    {TR_HOTKEY_BUILD_CLONE, "Klona byggnad under pekaren"},
    {TR_HOTKEY_LOAD_FILE, "Öppna fil"},
    {TR_HOTKEY_SAVE_FILE, "Spara fil"},
    {TR_HOTKEY_INCREASE_GAME_SPEED, "Öka spelhastighet"},
    {TR_HOTKEY_DECREASE_GAME_SPEED, "Minska spelhastighet"},
    {TR_HOTKEY_TOGGLE_PAUSE, "Pausa"},
    {TR_HOTKEY_CYCLE_LEGION, "Växla mellan legioner"},
    {TR_HOTKEY_ROTATE_MAP_LEFT, "Rotera kartan vänster"},
    {TR_HOTKEY_ROTATE_MAP_RIGHT, "Rotera kartan höger"},
    {TR_HOTKEY_SHOW_ADVISOR_LABOR, "Arbetarrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_MILITARY, "Militärrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_IMPERIAL, "Imperierådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_RATINGS, "Ställningsrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_TRADE, "Handlesrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_POPULATION, "Populationsrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_HEALTH, "Hälsorådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_EDUCATION, "Utbildningsrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, "Nöjesrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_RELIGION, "Religionsrådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_FINANCIAL, "Ekonomirådgivare"},
    {TR_HOTKEY_SHOW_ADVISOR_CHIEF, "Huvudrådgivare"},
    {TR_HOTKEY_TOGGLE_OVERLAY, "Slå på/av nuvarande överlägg"},
    {TR_HOTKEY_SHOW_OVERLAY_WATER, "Vattenöverlägg"},
    {TR_HOTKEY_SHOW_OVERLAY_FIRE, "Eldöverlägg"},
    {TR_HOTKEY_SHOW_OVERLAY_DAMAGE, "Skadeöverlägg"},
    {TR_HOTKEY_SHOW_OVERLAY_CRIME, "Brottsöverlägg"},
    {TR_HOTKEY_SHOW_OVERLAY_PROBLEMS, "Problemöverlägg"},
    {TR_HOTKEY_GO_TO_BOOKMARK_1, "Gå till bokmärke 1"},
    {TR_HOTKEY_GO_TO_BOOKMARK_2, "Gå till bokmärke 2"},
    {TR_HOTKEY_GO_TO_BOOKMARK_3, "Gå till bokmärke 3"},
    {TR_HOTKEY_GO_TO_BOOKMARK_4, "Gå till bokmärke 4"},
    {TR_HOTKEY_SET_BOOKMARK_1, "Sätt bokmärke 1"},
    {TR_HOTKEY_SET_BOOKMARK_2, "Sätt bokmärke 2"},
    {TR_HOTKEY_SET_BOOKMARK_3, "Sätt bokmärke 3"},
    {TR_HOTKEY_SET_BOOKMARK_4, "Sätt bokmärke 4"},
    {TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, "Slå på/av stridsinformation"},
    {TR_HOTKEY_EDIT_TITLE, "Nytt kortkommando"}
};

void translation_swedish(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
