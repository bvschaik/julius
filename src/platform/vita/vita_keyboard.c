#include "vita_keyboard.h"

#include "core/encoding.h"

#include <string.h>
#include <stdbool.h>
#include <psp2/apputil.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ime_dialog.h>
#include <psp2/message_dialog.h>
#include <vita2d.h>

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
static char ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];

static int init_ime_dialog(const char *title, const char *initial_text, int max_text_length, int type, int option)
{
    if (ime_dialog_running) {
        return -1;
    }

    // Convert UTF8 to UTF16
    memset(ime_title_utf16, 0, sizeof(ime_title_utf16));
    memset(ime_initial_text_utf16, 0, sizeof(ime_initial_text_utf16));
    encoding_utf8_to_utf16(title, ime_title_utf16);
    encoding_utf8_to_utf16(initial_text, ime_initial_text_utf16);

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
            encoding_utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
        } else {
            status = IME_DIALOG_RESULT_CANCELED;
        }

        sceImeDialogTerm();

        ime_dialog_running = 0;
    }

    return status;
}

char *vita_keyboard_get(const char *title, const char *initial_text, int max_len)
{
    char *name = NULL;

    if (ime_init_apputils == 0) {
        sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
        sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
        ime_init_apputils = 1;
    }
    init_ime_dialog(title, initial_text, max_len, SCE_IME_TYPE_BASIC_LATIN, 0);
    bool done = false;
    while (!done) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        done = true;

        int ime_result = update_ime_dialog();
        if (ime_result == IME_DIALOG_RESULT_FINISHED) {
            name = ime_input_text_utf8;
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
