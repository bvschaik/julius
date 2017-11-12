#include "message.h"

#include "core/time.h"

#include "PlayerMessage.h"

#define MAX_MESSAGE_CATEGORIES 20

static struct {
    struct {
        int pop500;
        int pop1000;
        int pop2000;
        int pop3000;
        int pop5000;
        int pop10000;
        int pop15000;
        int pop20000;
        int pop25000;
    } population_shown;

    int message_count[MAX_MESSAGE_CATEGORIES];
    int message_delay[MAX_MESSAGE_CATEGORIES];

    time_millis last_sound_time[MESSAGE_CAT_RIOT_COLLAPSE+1];
} data;

void city_message_init_scenario()
{
    for (int i = 0; i < MAX_MESSAGE_CATEGORIES; i++) {
        data.message_count[i] = 0;
        data.message_delay[i] = 0;
    }
    // population
    data.population_shown.pop500 = 0;
    data.population_shown.pop1000 = 0;
    data.population_shown.pop2000 = 0;
    data.population_shown.pop3000 = 0;
    data.population_shown.pop5000 = 0;
    data.population_shown.pop10000 = 0;
    data.population_shown.pop15000 = 0;
    data.population_shown.pop20000 = 0;
    data.population_shown.pop25000 = 0;

    for (int i = 0; i <= MESSAGE_CAT_RIOT_COLLAPSE; i++) {
        data.last_sound_time[i] = 0;
    }
}

void city_message_apply_sound_interval(message_category category)
{
    time_millis now = time_get_millis();
    if (now <= 15000 + data.last_sound_time[category]) {
        PlayerMessage_disableSoundForNextMessage();
    } else {
        data.last_sound_time[category] = now;
    }
}

void city_message_post_with_popup_delay(message_category category, int message_type, int param1, short param2)
{
    int use_popup = 0;
    if (data.message_delay[category] <= 0) {
        use_popup = 1;
        data.message_delay[category] = 12;
    }
    PlayerMessage_post(use_popup, message_type, param1, param2);
    data.message_count[category]++;
}

void city_message_post_with_message_delay(message_category category, int use_popup, int message_type, int delay)
{
    if (category == MESSAGE_CAT_FISHING_BLOCKED || category == MESSAGE_CAT_NO_WORKING_DOCK) {
        // bug in the original game: delays for 'fishing blocked' and 'no working dock'
        // are stored in message_count with manual countdown
        if (data.message_count[category] > 0) {
            data.message_count[category]--;
        } else {
            data.message_count[category] = delay;
            PlayerMessage_post(use_popup, message_type, 0, 0);
        }
    } else {
        if (data.message_delay[category] <= 0) {
            data.message_delay[category] = delay;
            PlayerMessage_post(use_popup, message_type, 0, 0);
        }
    }
}

void city_message_reset_category_count(message_category category)
{
    data.message_count[category] = 0;
}

void city_message_increase_category_count(message_category category)
{
    data.message_count[category]++;
}

int city_message_get_category_count(message_category category)
{
    return data.message_count[category];
}

void city_message_decrease_delays()
{
    for (int i = 0; i < MAX_MESSAGE_CATEGORIES; i++) {
        if (data.message_delay[i] > 0) {
            data.message_delay[i]--;
        }
    }
}

int city_message_mark_population_shown(int population)
{
    int *field = 0;
    switch (population) {
        case 500: field = &data.population_shown.pop500; break;
        case 1000: field = &data.population_shown.pop1000; break;
        case 2000: field = &data.population_shown.pop2000; break;
        case 3000: field = &data.population_shown.pop3000; break;
        case 5000: field = &data.population_shown.pop5000; break;
        case 10000: field = &data.population_shown.pop10000; break;
        case 15000: field = &data.population_shown.pop15000; break;
        case 20000: field = &data.population_shown.pop20000; break;
        case 25000: field = &data.population_shown.pop25000; break;
    }
    if (field && !*field) {
        *field = 1;
        return 1;
    }
    return 0;
}


void city_message_save_state(buffer *counts, buffer *delays, buffer *population)
{
    for (int i = 0; i < MAX_MESSAGE_CATEGORIES; i++) {
        buffer_write_i32(counts, data.message_count[i]);
        buffer_write_i32(delays, data.message_delay[i]);
    }
    // population
    buffer_write_u8(population, 0);
    buffer_write_u8(population, data.population_shown.pop500);
    buffer_write_u8(population, data.population_shown.pop1000);
    buffer_write_u8(population, data.population_shown.pop2000);
    buffer_write_u8(population, data.population_shown.pop3000);
    buffer_write_u8(population, data.population_shown.pop5000);
    buffer_write_u8(population, data.population_shown.pop10000);
    buffer_write_u8(population, data.population_shown.pop15000);
    buffer_write_u8(population, data.population_shown.pop20000);
    buffer_write_u8(population, data.population_shown.pop25000);
}

void city_message_load_state(buffer *counts, buffer *delays, buffer *population)
{
    for (int i = 0; i < MAX_MESSAGE_CATEGORIES; i++) {
        data.message_count[i] = buffer_read_i32(counts);
        data.message_delay[i] = buffer_read_i32(delays);
    }
    // population
    buffer_skip(population, 1);
    data.population_shown.pop500 = buffer_read_u8(population);
    data.population_shown.pop1000 = buffer_read_u8(population);
    data.population_shown.pop2000 = buffer_read_u8(population);
    data.population_shown.pop3000 = buffer_read_u8(population);
    data.population_shown.pop5000 = buffer_read_u8(population);
    data.population_shown.pop10000 = buffer_read_u8(population);
    data.population_shown.pop15000 = buffer_read_u8(population);
    data.population_shown.pop20000 = buffer_read_u8(population);
    data.population_shown.pop25000 = buffer_read_u8(population);
}
