#include "arguments.h"

#include "SDL.h"

#include <stdio.h>

#define CURSOR_SCALE_ERROR_MESSAGE "Option --cursor-scale must be followed by a scale value of 1, 1.5 or 2"
#define DISPLAY_SCALE_ERROR_MESSAGE "Option --display-scale must be followed by a scale value between 0.5 and 5"
#define WINDOWED_AND_FULLSCREEN_ERROR_MESSAGE "Option --windowed and --fullscreen cannot both be specified"
#define DISPLAY_ID_ERROR_MESSAGE "Option --display must be followed by a number indicating the display, starting from 0"
#define UNKNOWN_OPTION_ERROR_MESSAGE "Option %s not recognized"

static void print_log(const char *message)
{
    printf("%s\n", message);
}

static void print_log_str(const char *format, const char *value)
{
    printf(format, value);
    printf("\n");
}

static int parse_decimal_as_percentage(const char *str)
{
    const char *start = str;
    char *end;
    long whole = SDL_strtol(start, &end, 10);
    int percentage = 100 * (int) whole;
    if (*end == ',' || *end == '.') {
        end++;
        start = end;
        long fraction = SDL_strtol(start, &end, 10);
        switch (end - start) {
            case 0:
                break;
            case 1:
                percentage += fraction * 10;
                break;
            case 2:
                percentage += fraction;
                break;
            default: {
                int fraction_digits = (int) (end - start);
                while (fraction_digits > 2) {
                    fraction = fraction / 10;
                    fraction_digits--;
                }
                percentage += fraction;
                break;
            }
        }
    }
    if (*end) {
        // still some characters left, print out warning
        print_log_str("Invalid decimal: %s", str);
        return -1;
    }
    return percentage;
}

int platform_parse_arguments(int argc, char **argv, augustus_args *output_args)
{
    int ok = 1;
    int add_blank_line = 1;

    // Set sensible defaults
    output_args->data_directory = 0;
    output_args->display_scale_percentage = 0;
    output_args->cursor_scale_percentage = 0;
    output_args->force_windowed = 0;
    output_args->launch_asset_previewer = 0;
    output_args->enable_joysticks = 0;
    output_args->use_software_cursor = 0;
    output_args->force_fullscreen = 0;
    output_args->display_id = 0;

    for (int i = 1; i < argc; i++) {
        // we ignore "-psn" arguments, this is needed to launch the app
        // from the Finder on macOS.
        // https://hg.libsdl.org/SDL/file/c005c49beaa9/test/testdropfile.c#l47
        if (SDL_strncmp(argv[i], "-psn", 4) == 0) {
            continue;
        }
        if (SDL_strcmp(argv[i], "--display-scale") == 0) {
            if (i + 1 < argc) {
                int percentage = parse_decimal_as_percentage(argv[i + 1]);
                i++;
                if (percentage < 50 || percentage > 500) {
                    print_log(DISPLAY_SCALE_ERROR_MESSAGE);
                    ok = 0;
                } else {
                    output_args->display_scale_percentage = percentage;
                }
            } else {
                print_log(DISPLAY_SCALE_ERROR_MESSAGE);
                ok = 0;
            }
        } else if (SDL_strcmp(argv[i], "--cursor-scale") == 0) {
            if (i + 1 < argc) {
                int percentage = parse_decimal_as_percentage(argv[i + 1]);
                i++;
                if (percentage == 100 || percentage == 150 || percentage == 200) {
                    output_args->cursor_scale_percentage = percentage;
                } else {
                    print_log(CURSOR_SCALE_ERROR_MESSAGE);
                    ok = 0;
                }
            } else {
                print_log(CURSOR_SCALE_ERROR_MESSAGE);
                ok = 0;
            }
        } else if (SDL_strcmp(argv[i], "--display") == 0) {
            if (i + 1 < argc) {
                output_args->display_id = SDL_strtol(argv[i + 1], 0, 10);
                i++;
            } else {
                print_log(DISPLAY_ID_ERROR_MESSAGE);
                ok = 0;
            }
        } else if (SDL_strcmp(argv[i], "--windowed") == 0) {
            output_args->force_windowed = 1;
        } else if (SDL_strcmp(argv[i], "--asset-previewer") == 0) {
            output_args->launch_asset_previewer = 1;
        } else if (SDL_strcmp(argv[i], "--enable-joysticks") == 0) {
            output_args->enable_joysticks = 1;
        } else if (SDL_strcmp(argv[i], "--software-cursor") == 0) {
            output_args->use_software_cursor = 1;
        } else if (SDL_strcmp(argv[i], "--fullscreen") == 0) {
            output_args->force_fullscreen = 1;
        } else if (SDL_strcmp(argv[i], "--help") == 0) {
            add_blank_line = 0;
            ok = 0;
        } else if (SDL_strncmp(argv[i], "--", 2) == 0) {
            print_log_str(UNKNOWN_OPTION_ERROR_MESSAGE, argv[i]);
            ok = 0;
        } else {
            output_args->data_directory = argv[i];
        }
    }
    if (output_args->force_fullscreen && output_args->force_windowed) {
        print_log(WINDOWED_AND_FULLSCREEN_ERROR_MESSAGE);
        ok = 0;
    }

    if (!ok) {
        if (add_blank_line) {
            print_log("");
        }
        print_log("Usage: julius [ARGS] [DATA_DIR]");
        print_log("ARGS may be:");
        print_log("--display-scale NUMBER");
        print_log("          Scales the display by a factor of NUMBER. Number can be between 0.5 and 5");
        print_log("--cursor-scale NUMBER");
        print_log("          Scales the mouse cursor by a factor of NUMBER. Number can be 1, 1.5 or 2");
        print_log("--windowed");
        print_log("          Forces the game to start in windowed mode");
        print_log("--fullscreen");
        print_log("          Forces the game to start fullscreen");
        print_log("--display ID");
        print_log("          Forces the game to start on the specified display, numbered from 0");
        print_log("--asset-previewer");
        print_log("          Runs the extra asset previewer instead of the game");
        print_log("--enable-joysticks");
        print_log("          Enables joystick support");
        print_log("--software-cursor");
        print_log("          Uses a software cursor instead of the default hardware cursor");
        print_log("The last argument, if present, is interpreted as data directory for the Caesar 3 installation");
    }
    return ok;
}
