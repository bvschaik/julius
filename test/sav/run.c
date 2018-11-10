#include "core/time.h"
#include "game/file.h"
#include "game/game.h"
#include "game/settings.h"

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__)
#include <execinfo.h>
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "sav_compare.h"

static void handler(int sig)
{
#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__)
    void *array[100];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#else
    fprintf(stderr, "Oops, crashed with signal %d :(\n", sig);
#endif
    exit(1);
}

static void run_ticks(int ticks)
{
    setting_reset_speeds(100, setting_scroll_speed());
    time_set_millis(0);
    for (int i = 1; i <= ticks; i++) {
        time_set_millis(2 * i);
        game_run();
    }
}

static int run_autopilot(const char *input_saved_game, const char *output_saved_game, int ticks_to_run)
{
    printf("Running autopilot: %s --> %s in %d ticks\n", input_saved_game, output_saved_game, ticks_to_run);
    signal(SIGSEGV, handler);

    chdir("data/sav");
    if (!game_pre_init()) {
        printf("Unable to run Game_preInit\n");
        return 1;
    }

    if (!game_init()) {
        printf("Unable to run Game_init\n");
        return 2;
    }

    if (!game_file_load_saved_game(input_saved_game)) {
        char wd[500];
        getcwd(wd, 500);
        printf("Unable to load saved game from %s\n", wd);
        return 3;
    }
    run_ticks(ticks_to_run);
    printf("Saving game to %s\n", output_saved_game);
    game_file_write_saved_game(output_saved_game);
    printf("Done\n");

    game_exit();

    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        printf("Incorrect number of arguments (%d)\n", argc);
        return -1;
    }
    const char *input = argv[1];
    const char *output = argv[2];
    const char *expected = argv[3];
    int ticks = atoi(argv[4]);
    if (run_autopilot(input, output, ticks) == 0) {
        return compare_files(expected, output);
    } else {
        return 1;
    }
}
