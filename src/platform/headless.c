#include "core/config.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/lang.h"
#include "core/time.h"
#include "game/game.h"
#include "game/settings.h"
#include "game/system.h"
#include "platform/arguments.h"
#include "platform/file_manager.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static struct {
    int quit;
} data = { 0 };

static FILE *log_file = 0;

static void write_log(const char *level, const char *message)
{
    if (log_file) {
        fprintf(log_file, "%s: %s\n", level, message);
        fflush(log_file);
    }
}

static void setup_logging(void)
{
    log_file = fopen("julius-headless-log.txt", "wt");
    if (log_file) {
        write_log("INFO", "Headless mode logging initialized");
    }
}

static void teardown_logging(void)
{
    if (log_file) {
        write_log("INFO", "Shutting down");
        fclose(log_file);
    }
}

void system_exit(void)
{
    data.quit = 1;
}

static int pre_init(const char *custom_data_dir)
{
    if (custom_data_dir) {
        printf("Loading game from %s\n", custom_data_dir);
        if (!platform_file_manager_set_base_path(custom_data_dir)) {
            fprintf(stderr, "ERROR: %s: directory not found\n", custom_data_dir);
            return 0;
        }
        return game_pre_init();
    }

    printf("Loading game from working directory\n");
    if (game_pre_init()) {
        return 1;
    }

    fprintf(stderr, "ERROR: Julius requires the original files from Caesar 3 to run.\n");
    fprintf(stderr, "Run: julius_headless path-to-c3-directory\n");
    return 0;
}

static void run_ticks(int num_ticks)
{
    for (int i = 0; i < num_ticks && !data.quit; i++) {
        game_run();
    }
}

int main(int argc, char **argv)
{
    julius_args args;
    if (!platform_parse_arguments(argc, argv, &args)) {
        // Continue anyway for headless mode
    }

    signal(SIGSEGV, SIG_DFL); // Use default handler
    setup_logging();

    printf("Julius Headless version %s\n", system_version());

    const char *base_dir = args.data_directory;
    if (!pre_init(base_dir)) {
        fprintf(stderr, "Exiting: game pre-init failed\n");
        teardown_logging();
        exit(1);
    }

    if (!game_init()) {
        fprintf(stderr, "Exiting: game init failed\n");
        teardown_logging();
        exit(2);
    }

    printf("Game initialized successfully in headless mode\n");
    printf("Running main loop...\n");

    // Main headless loop - run game ticks as fast as possible
    unsigned long tick_count = 0;
    time_t start_time = time(NULL);

    while (!data.quit) {
        // Run one game tick
        run_ticks(1);
        tick_count++;

        // Print status every 1000 ticks
        if (tick_count % 1000 == 0) {
            time_t elapsed = time(NULL) - start_time;
            if (elapsed > 0) {
                printf("Ticks: %lu (%.1f ticks/sec)\n",
                       tick_count, (double)tick_count / elapsed);
            }
        }

        // For now, limit to 10000 ticks to avoid infinite loop in testing
        if (tick_count >= 10000) {
            printf("Reached tick limit, exiting\n");
            break;
        }

        // Small sleep to avoid burning CPU during testing
        usleep(1000); // 1ms
    }

    printf("Exiting game after %lu ticks\n", tick_count);
    game_exit();
    teardown_logging();

    return 0;
}
