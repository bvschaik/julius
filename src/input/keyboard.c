#include "keyboard.h"

#include "core/encoding.h"
#include "core/string.h"
#include "game/system.h"
#include "graphics/text.h"

static struct {
    int insert;
    int capture;
    int accepted;

    int capture_numeric;
    void (*capture_numeric_callback)(int);

    uint8_t *text;
    int editing_length;
    int cursor_position;
    int length;
    int max_length;
    int allow_punctuation;

    int viewport_start;
    int viewport_end;
    int viewport_cursor_position;

    int box_width;
    font_t font;
} data;

static int get_char_bytes(const uint8_t *str)
{
    return str[0] >= 0x80 && encoding_is_multibyte() ? 2 : 1;
}

static int get_current_char_bytes(void)
{
    return get_char_bytes(&data.text[data.cursor_position]);
}

static void set_viewport_to_start(void)
{
    data.viewport_start = 0;
    data.viewport_end = text_get_max_length_for_width(data.text, data.length, data.font, data.box_width, 0);
}

static void set_viewport_to_end(void)
{
    data.viewport_end = data.length;
    int maxlen = text_get_max_length_for_width(data.text, data.length, data.font, data.box_width, 1);
    data.viewport_start = data.length - maxlen;
}

static void include_cursor_in_viewport(void)
{
    // first check if we can keep the viewport
    int new_start = data.viewport_start;
    int new_end = text_get_max_length_for_width(data.text, data.length - new_start, data.font, data.box_width, 0);
    if (data.cursor_position >= new_start && data.cursor_position < new_end && new_start + new_end < data.length) {
        return;
    }
    if (data.cursor_position <= data.viewport_cursor_position) {
        // move toward start
        int maxlen = text_get_max_length_for_width(
            data.text + data.cursor_position,
            data.length - data.cursor_position,
            data.font, data.box_width, 0);
        if (data.cursor_position + maxlen < data.length) {
            data.viewport_start = data.cursor_position;
            data.viewport_end = data.cursor_position + maxlen;
        } else {
            // all remaining text fits: set to end
            set_viewport_to_end();
        }
    } else {
        // move toward end
        int viewport_length = data.cursor_position + get_current_char_bytes();
        int maxlen = text_get_max_length_for_width(
            data.text, viewport_length, data.font, data.box_width, 1);
        if (maxlen < viewport_length) {
            data.viewport_start = viewport_length - maxlen;
            data.viewport_end = viewport_length;
        } else {
            // all remaining text fits: set to start
            set_viewport_to_start();
        }
    }
}

static void update_viewport(int has_changed)
{
    int is_within_viewport = data.cursor_position >= data.viewport_start &&
        data.cursor_position < data.viewport_end;
    if (!has_changed && is_within_viewport) {
        // no update necessary
    } else if (data.cursor_position == 0) {
        set_viewport_to_start();
    } else if (data.cursor_position == data.length) {
        set_viewport_to_end();
    } else {
        include_cursor_in_viewport();
    }
    data.viewport_cursor_position = data.cursor_position;
}

void keyboard_start_capture(uint8_t *text, int max_length, int allow_punctuation, int box_width, font_t font)
{
    data.capture = 1;
    data.text = text;
    data.length = string_length(text);
    data.cursor_position = data.length;
    data.max_length = max_length;
    data.allow_punctuation = allow_punctuation;
    data.accepted = 0;
    data.box_width = box_width;
    data.font = font;
    update_viewport(1);
    system_start_text_input();
}

void keyboard_refresh(void)
{
    data.length = string_length(data.text);
    data.cursor_position = data.length;
    update_viewport(1);
}

void keyboard_resume_capture(void)
{
    data.capture = 1;
    system_start_text_input();
}

void keyboard_pause_capture(void)
{
    data.capture = 0;
    system_keyboard_hide();
    system_stop_text_input();
}

void keyboard_stop_capture(void)
{
    data.capture = 0;
    data.text = 0;
    data.cursor_position = 0;
    data.length = 0;
    data.max_length = 0;
    data.accepted = 0;
    system_keyboard_hide();
    system_stop_text_input();
}

void keyboard_start_capture_numeric(void (*callback)(int))
{
    data.capture_numeric = 1;
    data.capture_numeric_callback = callback;
    system_start_text_input();
}

void keyboard_stop_capture_numeric(void)
{
    data.capture_numeric = 0;
    data.capture_numeric_callback = 0;
    system_stop_text_input();
}

int keyboard_input_is_accepted(void)
{
    if (data.accepted) {
        data.accepted = 0;
        return 1;
    } else {
        return 0;
    }
}

int keyboard_is_insert(void)
{
    return data.insert;
}

int keyboard_is_capturing(void)
{
    return data.capture;
}

int keyboard_cursor_position(void)
{
    return data.cursor_position - data.viewport_start;
}

int keyboard_offset_start(void)
{
    return data.viewport_start;
}

int keyboard_offset_end(void)
{
    return data.viewport_end;
}

void keyboard_return(void)
{
    data.accepted = 1;
}

static void move_left(uint8_t *start, const uint8_t *end)
{
    while (start < end) {
        start[0] = start[1];
        start++;
    }
    *start = 0;
}

static void move_right(const uint8_t *start, uint8_t *end)
{
    end[1] = 0;
    while (end > start) {
        end--;
        end[1] = end[0];
    }
}

static void move_cursor_left(void)
{
    if (encoding_is_multibyte()) {
        int i = 0;
        int bytes = 0;
        while (i + bytes < data.cursor_position) {
            i += bytes;
            bytes = data.text[i] >= 0x80 ? 2 : 1;
        }
        data.cursor_position = i;
    } else {
        data.cursor_position--;
    }
}

static void move_cursor_right(void)
{
    data.cursor_position += get_current_char_bytes();
}

static void insert_char(const uint8_t *value, int bytes)
{
    if (data.length + bytes == data.max_length) {
        return;
    }
    for (int i = 0; i < bytes; i++) {
        move_right(&data.text[data.cursor_position], &data.text[data.length]);
        data.text[data.cursor_position] = value[i];
        data.cursor_position++;
    }
    data.length += bytes;
}

static void remove_current_char(void)
{
    int bytes = get_current_char_bytes();
    for (int i = 0; i < bytes; i++) {
        move_left(&data.text[data.cursor_position], &data.text[data.length]);
    }
    data.length -= bytes;
}

static void add_char(const uint8_t *value, int bytes)
{
    if (data.insert) {
        insert_char(value, bytes);
    } else {
        if (data.cursor_position < data.length) {
            remove_current_char();
        }
        insert_char(value, bytes);
    }
}

void keyboard_backspace(void)
{
    if (data.capture && data.cursor_position > 0) {
        move_cursor_left();
        remove_current_char();
        update_viewport(1);
    }
}

void keyboard_delete(void)
{
    if (data.capture && data.cursor_position < data.length) {
        remove_current_char();
        update_viewport(1);
    }
}

void keyboard_insert(void)
{
    data.insert ^= 1;
}

void keyboard_left(void)
{
    if (data.capture) {
        if (data.cursor_position > 0) {
            move_cursor_left();
            update_viewport(0);
        }
    }
}

void keyboard_right(void)
{
    if (data.capture) {
        if (data.cursor_position < data.length) {
            move_cursor_right();
            update_viewport(0);
        }
    }
}

void keyboard_home(void)
{
    if (data.capture) {
        data.cursor_position = 0;
        update_viewport(0);
    }
}

void keyboard_end(void)
{
    if (data.capture) {
        data.cursor_position = data.length;
        update_viewport(0);
    }
}

static int keyboard_character(uint8_t *text)
{
    uint8_t c = text[0];

    int add = 0;
    if (c == ' ' || c == '-') {
        add = 1;
    } else if (c >= '0' && c <= '9') {
        add = 1;
    } else if (c >= 'a' && c <= 'z') {
        add = 1;
    } else if (c >= 'A' && c <= 'Z') {
        add = 1;
    } else if (c == ',' || c == '.' || c == '?' || c == '!') {
        add = data.allow_punctuation;
    } else if (c >= 0x80) { // do not check non-ascii for valid characters
        add = 1;
    }

    int bytes = get_char_bytes(text);
    if (add) {
        add_char(text, bytes);
        update_viewport(1);
    }
    return bytes;
}

void keyboard_editing_text(const char *text_utf8)
{
    for (int i = 0; i < data.editing_length; i++) {
        keyboard_backspace();
    }
    data.editing_length = keyboard_text(text_utf8);
}

int keyboard_text(const char *text_utf8)
{
    data.editing_length = 0;
    if (data.capture_numeric) {
        char c = text_utf8[0];
        if (c >= '0' && c <= '9') {
            data.capture_numeric_callback(c - '0');
        }
        return 1;
    }
    if (!data.capture) {
        return 1;
    }

    uint8_t internal_char[100];
    encoding_from_utf8(text_utf8, internal_char, 100);

    int index = 0;
    int length = 0;
    while (internal_char[index]) {
        index += keyboard_character(&internal_char[index]);
        length++;
    }
    return length;
}

const uint8_t *keyboard_get_text(void)
{
    return data.text;
}

void keyboard_set_text(const uint8_t *text)
{
    if (!data.capture) {
        return;
    }
    string_copy(text, data.text, data.max_length);
    keyboard_refresh();
}

int keyboard_get_max_text_length(void)
{
    return data.max_length;
}
