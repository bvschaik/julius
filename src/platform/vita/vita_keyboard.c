#include <string.h>
#include <stdbool.h>
#include <psp2/apputil.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ime_dialog.h>
#include <psp2/message_dialog.h>
#include <vita2d.h>
#include "vita_keyboard.h"

#define IME_DIALOG_RESULT_NONE 0
#define IME_DIALOG_RESULT_RUNNING 1
#define IME_DIALOG_RESULT_FINISHED 2
#define IME_DIALOG_RESULT_CANCELED 3

static int ime_dialog_running = 0;
static int ime_dialog_option = 0;
static int ime_init_apputils = 0;

static uint16_t ime_title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t ime_initial_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t ime_input_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static uint8_t ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];

static void utf16_to_utf8(uint16_t *src, uint8_t *dst)
{
    int i;
    for (i = 0; src[i]; i++) {
        if ((src[i] & 0xFF80) == 0) {
            *(dst++) = src[i] & 0xFF;
        } else if((src[i] & 0xF800) == 0) {
            *(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
            *(dst++) = (src[i] & 0x3F) | 0x80;
        } else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
            *(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
            *(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
            *(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
            *(dst++) = (src[i + 1] & 0x3F) | 0x80;
            i += 1;
        } else {
            *(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
            *(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
            *(dst++) = (src[i] & 0x3F) | 0x80;
        }
    }

    *dst = '\0';
}

static void utf8_to_utf16(uint8_t *src, uint16_t *dst)
{
    int i;
    for (i = 0; src[i];) {
        if ((src[i] & 0xE0) == 0xE0) {
            *(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
            i += 3;
        } else if ((src[i] & 0xC0) == 0xC0) {
            *(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
            i += 2;
        } else {
            *(dst++) = src[i];
            i += 1;
        }
    }

    *dst = '\0';
}

static int init_ime_dialog(char *title, const char *initial_text, int max_text_length, int type, int option)
{
    if (ime_dialog_running) {
        return -1;
    }

    // Convert UTF8 to UTF16
    memset(ime_title_utf16, 0, sizeof(ime_title_utf16));
    memset(ime_initial_text_utf16, 0, sizeof(ime_initial_text_utf16));
    utf8_to_utf16((uint8_t *)title, ime_title_utf16);
    utf8_to_utf16((uint8_t *)initial_text, ime_initial_text_utf16);

    //clear previous results
    memset(ime_input_text_utf16, 0, sizeof(ime_input_text_utf16));
    memset(ime_input_text_utf8, 0, sizeof(ime_input_text_utf8));

    SceImeDialogParam param;
    sceImeDialogParamInit(&param);

    param.supportedLanguages = 0x0001FFFF;
    param.languagesForced = SCE_TRUE;
    param.type = type;
    param.option = option;
    if (option == SCE_IME_OPTION_MULTILINE) {
        param.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
    }
    param.title = ime_title_utf16;
    param.maxTextLength = max_text_length;
    param.initialText = ime_initial_text_utf16;
    param.inputTextBuffer = ime_input_text_utf16;

    int res = sceImeDialogInit(&param);
    if (res >= 0) {
        ime_dialog_running = 1;
        ime_dialog_option = option;
    }

    return res;
}

static uint8_t *get_ime_dialog_input_text_utf8(void)
{
    return ime_input_text_utf8;
}

static int update_ime_dialog(void) {
    if (!ime_dialog_running) {
        return IME_DIALOG_RESULT_NONE;
    }

    SceCommonDialogStatus status = sceImeDialogGetStatus();
    if (status == IME_DIALOG_RESULT_FINISHED) {
        SceImeDialogResult result;
        memset(&result, 0, sizeof(SceImeDialogResult));
        sceImeDialogGetResult(&result);

        if ((ime_dialog_option == SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_CLOSE) ||
        (ime_dialog_option != SCE_IME_OPTION_MULTILINE && (result.button == SCE_IME_DIALOG_BUTTON_ENTER || result.button == SCE_IME_DIALOG_BUTTON_CLOSE))) {
            // Convert UTF16 to UTF8
            utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
        } else {
            status = IME_DIALOG_RESULT_CANCELED;
        }

        sceImeDialogTerm();

        ime_dialog_running = 0;
    }

    return status;
}

char *vita_keyboard_get(char *title, const char *initial_text, int maxLen, int multiline)
{
    char *name = NULL;

    if (ime_init_apputils == 0) {
        sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
        sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
        ime_init_apputils = 1;
    }
    if (multiline) {
        init_ime_dialog(title, initial_text, maxLen, SCE_IME_TYPE_BASIC_LATIN, SCE_IME_OPTION_MULTILINE);
    } else {
        init_ime_dialog(title, initial_text, maxLen, SCE_IME_TYPE_BASIC_LATIN, 0);
    }
    bool done = false;
    while (!done) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        done = true;

        int ime_result = update_ime_dialog();
        if (ime_result == IME_DIALOG_RESULT_FINISHED) {
            name = (char *)get_ime_dialog_input_text_utf8();
        } else if (ime_result != IME_DIALOG_RESULT_CANCELED) {
            done = false;
        }

        vita2d_end_drawing();
        vita2d_common_dialog_update();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
    }
    return name;
}
