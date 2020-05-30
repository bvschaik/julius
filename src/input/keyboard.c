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
    int cursor_position;
    int length;
    int max_length;
    int allow_punctuation;

    int offset_start;
    int offset_end;

    int box_width;
    font_t font;
} data;

static void change_start_offset_by(int length)
{
    if (length < 0 || data.offset_start == 0) {
        return;
    }
    int max_left = (data.offset_start < length) ? data.offset_start : length;
    data.offset_start -= max_left;
    data.offset_end = data.offset_start + text_get_max_length_for_width(data.text + data.offset_start, data.length, data.font, data.box_width, 0);
}

static void change_end_offset_by(int length)
{
    if (length < 0 || data.offset_end == data.length) {
        return;
    }
    int remaining = data.length - data.offset_end;
    int max_right = (remaining < length) ? remaining : length;
    data.offset_end += max_right;
    data.offset_start = data.offset_end - text_get_max_length_for_width(data.text, data.offset_end, data.font, data.box_width, 1);
}

static void set_offset_to_end(void)
{
    data.offset_end = data.length;
    int maxlen = data.length;
    if (text_get_width(data.text, data.font) <= data.box_width) {
        data.offset_start = 0;
    } else {
        maxlen = text_get_max_length_for_width(data.text, data.length, data.font, data.box_width, 1);
        data.offset_start = data.length - maxlen;
    }
}

void keyboard_start_capture(uint8_t *text, int max_length, int allow_punctuation, const input_box *capture_box, font_t font)
{
    data.capture = 1;
    data.text = text;
    data.length = string_length(text);
    data.cursor_position = data.length;
    data.max_length = max_length;
    data.allow_punctuation = allow_punctuation;
    data.accepted = 0;
    data.box_width = (capture_box->width_blocks - 2) * INPUT_BOX_BLOCK_SIZE;
    data.font = font;
    set_offset_to_end();
}

void keyboard_refresh(void)
{
    data.length = string_length(data.text);
    data.cursor_position = data.length;
    set_offset_to_end();
}

void keyboard_resume_capture(void)
{
    data.capture = 1;
}

void keyboard_pause_capture(void)
{
    data.capture = 0;
    system_keyboard_hide();
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
}

void keyboard_start_capture_numeric(void (*callback)(int))
{
    data.capture_numeric = 1;
    data.capture_numeric_callback = callback;
}

void keyboard_stop_capture_numeric(void)
{
    data.capture_numeric = 0;
    data.capture_numeric_callback = 0;
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

int keyboard_cursor_position(void)
{
    return data.cursor_position - data.offset_start;
}

int keyboard_offset_start(void)
{
    return data.offset_start;
}

int keyboard_offset_end(void)
{
    return data.offset_end;
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
    *start = 0; // TODO ? trailing \0 should have been copied...
}

static void move_right(const uint8_t *start, uint8_t *end)
{
    end[1] = 0;
    while (end > start) {
        end--;
        end[1] = end[0];
    }
}

static void add_char(uint8_t value)
{
    if (data.insert) {
        if (data.length + 1 == data.max_length) {
            return;
        }
        move_right(&data.text[data.cursor_position], &data.text[data.length]);
        data.text[data.cursor_position] = value;
        data.cursor_position++;
        data.length++;
        data.offset_end++;
    } else {
        if (data.cursor_position == data.length && data.length + 1 == data.max_length) {
            return;
        }
        data.text[data.cursor_position] = value;
        data.cursor_position++;
        if (data.cursor_position >= data.length) {
            data.text[data.cursor_position] = 0;
            if (data.cursor_position > data.length) {
                data.length++;
                data.offset_end++;
            }
        }
    }
    int maxlen = text_get_max_length_for_width(data.text + data.offset_start, data.length, data.font, data.box_width, 0);
    if (data.cursor_position < (data.offset_start + maxlen - 2)) {
        data.offset_end = data.offset_start + maxlen;
        if (data.offset_end > data.length) {
            set_offset_to_end();
        }
    } else {
        if (!data.insert && data.offset_end < data.length) {
            data.offset_end++;
        }
        data.offset_start = data.offset_end - text_get_max_length_for_width(data.text, data.offset_end, data.font, data.box_width, 1);
    }
}

static void remove_current_char(void)
{
    move_left(&data.text[data.cursor_position], &data.text[data.length]);
    if (data.offset_end == data.length) {
        data.offset_end--;
    }
    data.length--;
    if (data.offset_end != data.length) {
        data.offset_end = data.offset_start + text_get_max_length_for_width(data.text + data.offset_start, data.length - data.offset_start, data.font, data.box_width, 0);
    } else if (data.offset_start) {
        data.offset_start = data.offset_end - text_get_max_length_for_width(data.text, data.offset_end, data.font, data.box_width, 1);
    }
}

void keyboard_backspace(void)
{
    if (data.capture && data.cursor_position > 0) {
        data.cursor_position--;
        remove_current_char();
    }
}

void keyboard_delete(void)
{
    if (data.capture && data.cursor_position < data.length) {
        remove_current_char();
    }
}

void keyboard_insert(void)
{
    data.insert ^= 1;
}

void keyboard_left(void)
{
    if (data.capture && data.cursor_position > 0) {
        data.cursor_position--;
    }
    if (data.offset_start > 0 && data.cursor_position == (data.offset_start - 1)) {
        change_start_offset_by(5);
    }
}

void keyboard_right(void)
{
    if (data.capture) {
        if (data.cursor_position < data.length) {
            data.cursor_position++;
        }
        if (data.offset_end < data.length && data.cursor_position == data.offset_end) {
            change_end_offset_by(5);
        }
    }
}

void keyboard_home(void)
{
    if (data.capture) {
        data.cursor_position = 0;
        if (data.offset_start > 0) {
            data.offset_start = 0;
            data.offset_end = text_get_max_length_for_width(data.text, data.length, data.font, data.box_width, 0);
        }
    }
}

void keyboard_end(void)
{
    if (data.capture) {
        data.cursor_position = data.length;
        if (data.offset_end < data.length) {
            set_offset_to_end();
        }
    }
}

void keyboard_character(const char *text_utf8)
{
    if (data.capture_numeric) {
        char c = text_utf8[0];
        if (c >= '0' && c <= '9') {
            data.capture_numeric_callback(c - '0');
        }
        return;
    }
    if (!data.capture) {
        return;
    }

    uint8_t internal_char[2];
    encoding_from_utf8(text_utf8, internal_char, 2);
    uint8_t c = internal_char[0];

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
    } else if (c >= 128) { // do not check non-ascii for valid characters
        add = 1;
    }

    if (add) {
        add_char(c);
    }
}
