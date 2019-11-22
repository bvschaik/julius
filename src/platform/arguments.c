#include "arguments.h"

#include "SDL.h"

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
                int fraction_digits = end - start;
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
        SDL_Log("Invalid decimal: %s\n", str);
        return -1;
    }
    SDL_Log("Percentage: %d", percentage);
    return percentage;
}

int platform_parse_arguments(int argc, char **argv, julius_args *output_args)
{
    int ok = 1;

    // Set sensible defaults
    output_args->data_directory = 0;
    output_args->display_scale_percentage = 100;

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
                    SDL_Log("Option --display-scale must be followed by a scale value between 0.5 and 5");
                    ok = 0;
                } else {
                    output_args->display_scale_percentage = percentage;
                }
            } else {
                SDL_Log("Option --display-scale must be followed by a scale value between 0.5 and 5");
                ok = 0;
            }
        } else {
            output_args->data_directory = argv[i];
        }
    }
    return ok;
}
