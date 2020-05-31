#include "message.h"

#include "core/encoding.h"
#include "core/file.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "figure/formation.h"
#include "game/time.h"
#include "graphics/window.h"
#include "sound/effect.h"
#include "window/message_dialog.h"

#define MAX_MESSAGES 1000
#define MAX_QUEUE 20
#define MAX_MESSAGE_CATEGORIES 20

static struct {
    city_message messages[MAX_MESSAGES];

    int queue[20];
    int consecutive_message_delay;

    int next_message_sequence;
    int total_messages;
    int current_message_id;

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

    int problem_count;
    int problem_index;
    time_millis problem_last_click_time;

    int scroll_position;
} data;

static int should_play_sound = 1;

void city_message_init_scenario(void)
{
    for (int i = 0; i < MAX_MESSAGES; i++) {
        data.messages[i].message_type = 0;
    }
    for (int i = 0; i < MAX_QUEUE; i++) {
        data.queue[i] = 0;
    }
    data.consecutive_message_delay = 0;

    data.next_message_sequence = 0;
    data.total_messages = 0;
    data.current_message_id = 0;

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

    city_message_init_problem_areas();
}

void city_message_init_problem_areas(void)
{
    data.problem_count = 0;
    data.problem_index = 0;
    data.problem_last_click_time = time_get_millis();
}

static int new_message_id(void)
{
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (!data.messages[i].message_type) {
            return i;
        }
    }
    return -1;
}

static int has_video(int text_id)
{
    const lang_message *msg = lang_get_message(text_id);
    if (!msg->video.text) {
        return 0;
    }
    char video_file[FILE_NAME_MAX];
    encoding_to_utf8(msg->video.text, video_file, FILE_NAME_MAX, 0);
    return file_exists(video_file, MAY_BE_LOCALIZED);
}

static void enqueue_message(int sequence)
{
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (!data.queue[i]) {
            data.queue[i] = sequence;
            break;
        }
    }
}

static void play_sound(int text_id)
{
    if (lang_get_message(text_id)->urgent == 1) {
        sound_effect_play(SOUND_EFFECT_FANFARE_URGENT);
    } else {
        sound_effect_play(SOUND_EFFECT_FANFARE);
    }
}

static void show_message_popup(int message_id)
{
    city_message *msg = &data.messages[message_id];
    data.consecutive_message_delay = 5;
    msg->is_read = 1;
    int text_id = city_message_get_text_id(msg->message_type);
    if (!has_video(text_id)) {
        play_sound(text_id);
    }
    window_message_dialog_show_city_message(text_id,
        msg->year, msg->month, msg->param1, msg->param2,
        city_message_get_advisor(msg->message_type), 1);
}

void city_message_disable_sound_for_next_message(void)
{
    should_play_sound = 0;
}

void city_message_apply_sound_interval(message_category category)
{
    time_millis now = time_get_millis();
    if (now - data.last_sound_time[category] <= 15000) {
        city_message_disable_sound_for_next_message();
    } else {
        data.last_sound_time[category] = now;
    }
}

void city_message_post(int use_popup, int message_type, int param1, int param2)
{
    int id = new_message_id();
    if (id < 0) {
        return;
    }
    data.total_messages++;
    data.current_message_id = id;

    city_message *msg = &data.messages[id];
    msg->message_type = message_type;
    msg->is_read = 0;
    msg->year = game_time_year();
    msg->month = game_time_month();
    msg->param1 = param1;
    msg->param2 = param2;
    msg->sequence = data.next_message_sequence++;

    int text_id = city_message_get_text_id(message_type);
    lang_message_type lang_msg_type = lang_get_message(text_id)->message_type;
    if (lang_msg_type == MESSAGE_TYPE_DISASTER || lang_msg_type == MESSAGE_TYPE_INVASION) {
        data.problem_count = 1;
        window_invalidate();
    }
    if (use_popup && window_is(WINDOW_CITY)) {
        show_message_popup(id);
    } else if (use_popup) {
        // add to queue to be processed when player returns to city
        enqueue_message(msg->sequence);
    } else if (should_play_sound) {
        play_sound(text_id);
    }
    should_play_sound = 1;
}

void city_message_post_with_popup_delay(message_category category, int message_type, int param1, short param2)
{
    int use_popup = 0;
    if (data.message_delay[category] <= 0) {
        use_popup = 1;
        data.message_delay[category] = 12;
    }
    city_message_post(use_popup, message_type, param1, param2);
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
            city_message_post(use_popup, message_type, 0, 0);
        }
    } else {
        if (data.message_delay[category] <= 0) {
            data.message_delay[category] = delay;
            city_message_post(use_popup, message_type, 0, 0);
        }
    }
}

void city_message_process_queue(void)
{
    if (data.consecutive_message_delay > 0) {
        data.consecutive_message_delay--;
        return;
    }
    int sequence = 0;
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (data.queue[i]) {
            sequence = data.queue[i];
            data.queue[i] = 0;
            break;
        }
    }
    if (sequence == 0) {
        return;
    }
    int message_id = -1;
    for (int i = 0; i < 999; i++) {
        if (!data.messages[i].message_type) {
            return;
        }
        if (data.messages[i].sequence == sequence) {
            message_id = i;
            break;
        }
    }
    if (message_id >= 0) {
        show_message_popup(message_id);
    }
}


void city_message_sort_and_compact(void)
{
    for (int i = 0; i < MAX_MESSAGES; i++) {
        for (int a = 0; a < MAX_MESSAGES - 1; a++) {
            int swap = 0;
            if (data.messages[a].message_type) {
                if (data.messages[a].sequence < data.messages[a+1].sequence) {
                    if (data.messages[a+1].message_type) {
                        swap = 1;
                    }
                }
            } else if (data.messages[a+1].message_type) {
                swap = 1;
            }
            if (swap) {
                city_message tmp_message = data.messages[a];
                data.messages[a] = data.messages[a+1];
                data.messages[a+1] = tmp_message;
            }
        }
    }
    data.total_messages = 0;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (data.messages[i].message_type) {
            data.total_messages++;
        }
    }
}

int city_message_get_text_id(city_message_type message_type)
{
    if (message_type > 50) {
        return message_type + 199;
    } else {
        return message_type + 99;
    }
}

message_advisor city_message_get_advisor(city_message_type message_type)
{
    switch (message_type) {
        case MESSAGE_LOCAL_UPRISING:
        case MESSAGE_BARBARIAN_ATTACK:
        case MESSAGE_CAESAR_ARMY_ATTACK:
        case MESSAGE_DISTANT_BATTLE:
        case MESSAGE_ENEMIES_CLOSING:
        case MESSAGE_ENEMIES_AT_THE_DOOR:
            return MESSAGE_ADVISOR_MILITARY;

        case MESSAGE_CAESAR_REQUESTS_GOODS:
        case MESSAGE_CAESAR_REQUESTS_MONEY:
        case MESSAGE_CAESAR_REQUESTS_ARMY:
        case MESSAGE_REQUEST_REMINDER:
        case MESSAGE_REQUEST_RECEIVED:
        case MESSAGE_REQUEST_REFUSED:
        case MESSAGE_REQUEST_REFUSED_OVERDUE:
        case MESSAGE_REQUEST_RECEIVED_LATE:
        case MESSAGE_REQUEST_CAN_COMPLY:
            return MESSAGE_ADVISOR_IMPERIAL;

        case MESSAGE_UNEMPLOYMENT:
        case MESSAGE_WORKERS_NEEDED:
        case MESSAGE_ROME_LOWERS_WAGES:
        case MESSAGE_ROME_RAISES_WAGES:
            return MESSAGE_ADVISOR_LABOR;

        case MESSAGE_NOT_ENOUGH_FOOD:
        case MESSAGE_FOOD_NOT_DELIVERED:
            return MESSAGE_ADVISOR_POPULATION;

        case MESSAGE_HEALTH_ILLNESS:
        case MESSAGE_HEALTH_DISEASE:
        case MESSAGE_HEALTH_PESTILENCE:
            return MESSAGE_ADVISOR_HEALTH;

        default:
            return MESSAGE_ADVISOR_NONE;
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

void city_message_decrease_delays(void)
{
    for (int i = 0; i < MAX_MESSAGE_CATEGORIES; i++) {
        if (data.message_delay[i] > 0) {
            data.message_delay[i]--;
        }
    }
}

int city_message_mark_population_shown(int population)
{
    int *field;
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
        default: return 0;
    }
    if (!*field) {
        *field = 1;
        return 1;
    }
    return 0;
}

const city_message *city_message_get(int message_id)
{
    return &data.messages[message_id];
}

int city_message_set_current(int message_id)
{
    return data.current_message_id = message_id;
}

void city_message_mark_read(int message_id)
{
    data.messages[message_id].is_read = 1;
}

void city_message_delete(int message_id)
{
    data.messages[message_id].message_type = 0;
    city_message_sort_and_compact();
}

int city_message_count(void)
{
    return data.total_messages;
}

int city_message_problem_area_count(void)
{
    return data.problem_count;
}

static int has_problem_area(const city_message *msg, lang_message_type lang_msg_type)
{
    if (lang_msg_type == MESSAGE_TYPE_DISASTER) {
        return 1;
    }
    if (lang_msg_type == MESSAGE_TYPE_INVASION) {
        if (formation_grid_offset_for_invasion(msg->param1)) {
            return 1;
        }
        // Formations have not been updated yet because the invasion just started.
        // Invasions always start at the end of the month: return true when we're in
        // the next month
        return (msg->month + 1) % 12 == game_time_month() &&
            msg->year + (msg->month + 1) / 12 == game_time_year();
    }
    return 0;
}

int city_message_next_problem_area_grid_offset(void)
{
    time_millis now = time_get_millis();
    if (now - data.problem_last_click_time > 3000) {
        data.problem_index = 0;
    }
    data.problem_last_click_time = now;

    city_message_sort_and_compact();
    data.problem_count = 0;
    for (int i = 0; i < 999; i++) {
        city_message *msg = &data.messages[i];
        if (msg->message_type && msg->year >= game_time_year() - 1) {
            const lang_message *lang_msg = lang_get_message(city_message_get_text_id(msg->message_type));
            lang_message_type lang_msg_type = lang_msg->message_type;
            if (has_problem_area(msg, lang_msg_type)) {
                data.problem_count++;
            }
        }
    }
    if (data.problem_count <= 0) {
        data.problem_index = 0;
        return 0;
    }
    if (data.problem_index >= data.problem_count) {
        data.problem_index = 0;
    }
    int index = 0;
    int current_year = game_time_year();
    for (int i = 0; i < 999; i++) {
        city_message *msg = &data.messages[i];
        if (msg->message_type && msg->year >= current_year - 1) {
            int text_id = city_message_get_text_id(msg->message_type);
            lang_message_type lang_msg_type = lang_get_message(text_id)->message_type;
            if (has_problem_area(msg, lang_msg_type)) {
                index++;
                if (data.problem_index < index) {
                    data.problem_index++;
                    int grid_offset = msg->param2;
                    if (lang_msg_type == MESSAGE_TYPE_INVASION) {
                        int formation_grid_offset = formation_grid_offset_for_invasion(msg->param1);
                        if (formation_grid_offset) {
                            grid_offset = formation_grid_offset;
                        }
                    }
                    return grid_offset;
                }
            }
        }
    }
    return 0;
}

void city_message_clear_scroll(void)
{
    data.scroll_position = 0;
}

int city_message_scroll_position(void)
{
    return data.scroll_position;
}

void city_message_set_scroll_position(int scroll_position)
{
    data.scroll_position = scroll_position;
}

void city_message_save_state(buffer *messages, buffer *extra, buffer *counts, buffer *delays, buffer *population)
{
    for (int i = 0; i < MAX_MESSAGES; i++) {
        city_message *msg = &data.messages[i];
        buffer_write_i32(messages, msg->param1);
        buffer_write_i16(messages, msg->year);
        buffer_write_i16(messages, msg->param2);
        buffer_write_i16(messages, msg->message_type);
        buffer_write_i16(messages, msg->sequence);
        buffer_write_u8(messages, msg->is_read);
        buffer_write_u8(messages, msg->month);
        buffer_write_i16(messages, 0);
    }

    buffer_write_i32(extra, data.next_message_sequence);
    buffer_write_i32(extra, data.total_messages);
    buffer_write_i32(extra, data.current_message_id);

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

void city_message_load_state(buffer *messages, buffer *extra, buffer *counts, buffer *delays, buffer *population)
{
    for (int i = 0; i < MAX_MESSAGES; i++) {
        city_message *msg = &data.messages[i];
        msg->param1 = buffer_read_i32(messages);
        msg->year = buffer_read_i16(messages);
        msg->param2 = buffer_read_i16(messages);
        msg->message_type = buffer_read_i16(messages);
        msg->sequence = buffer_read_i16(messages);
        msg->is_read = buffer_read_u8(messages);
        msg->month = buffer_read_u8(messages);
        buffer_skip(messages, 2);
    }

    data.next_message_sequence = buffer_read_i32(extra);
    data.total_messages = buffer_read_i32(extra);
    data.current_message_id = buffer_read_i32(extra);

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
