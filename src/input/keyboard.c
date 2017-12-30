#include "keyboard.h"

#include "core/string.h"

#include "Widget.h"

static struct {
    int insert;
    int capture;
    int accepted;
    
    uint8_t *text;
    int cursor_position;
    int length;
    int max_length;
    int allow_punctuation;
    
    int box_width;
    font_t font;
} data = {0};

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
}

void keyboard_refresh()
{
    data.length = string_length(data.text);
    data.cursor_position = data.length;
}

void keyboard_stop_capture()
{
    data.capture = 0;
    data.text = 0;
    data.cursor_position = 0;
    data.length = 0;
    data.max_length = 0;
    data.accepted = 0;
}

int keyboard_input_is_accepted()
{
    return data.accepted;
}

int keyboard_is_insert()
{
    return data.insert;
}

int keyboard_cursor_position()
{
    return data.cursor_position;
}

void keyboard_return()
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
    if (data.length + 1 == data.max_length) {
        return;
    }
    if (Widget_Text_getWidth(data.text, data.font) >= data.box_width) {
        return;
    }
    if (data.insert) {
        move_right(&data.text[data.cursor_position], &data.text[data.length]);
        data.text[data.cursor_position] = value;
        data.cursor_position++;
        data.length++;
    } else {
        data.text[data.cursor_position] = value;
        data.cursor_position++;
        if (data.cursor_position >= data.length) {
            data.text[data.cursor_position] = 0;
            data.length++;
        }
    }
}

static void remove_current_char()
{
    move_left(&data.text[data.cursor_position], &data.text[data.length]);
    data.length--;
}

void keyboard_backspace()
{
    if (data.capture && data.cursor_position > 0) {
        data.cursor_position--;
        remove_current_char();
    }
}

void keyboard_delete()
{
    if (data.capture && data.length > 0) {
        remove_current_char();
    }
}

void keyboard_insert()
{
    data.insert ^= 1;
}

void keyboard_left()
{
    if (data.capture && data.cursor_position > 0) {
        data.cursor_position--;
    }
}

void keyboard_right()
{
    if (data.capture && data.cursor_position < data.length) {
        data.cursor_position++;
    }
}

void keyboard_home()
{
    if (data.capture) {
        data.cursor_position = 0;
    }
}

void keyboard_end()
{
    data.cursor_position = data.length;
}

static uint8_t unicode_to_cp1252(int c)
{
    if (c == 0x152) { // OE
        return 140;
    } else if (c == 0x153) { // oe
        return 156;
    } else if (c <= 0xff) {
        // ascii + ISO-8859-1
        return (uint8_t) c;
    }
    return 0;
}

void keyboard_character(int unicode)
{
    if (!data.capture) {
        return;
    }
    int add = 0;
    uint8_t c = unicode_to_cp1252(unicode);
    // TODO correct ranges (original ones are from CP437 / dos extended ascii)
    if (c == ' ' || c == '-') {
        add = 1;
    } else if (c >= '0' && c <= '9') {
        add = 1;
    } else if (c >= 'a' && c <= 'z') {
        add = 1;
    } else if (c >= 'A' && c <= 'Z') {
        add = 1;
    } else if (c >= 128 && c <= 154) {
        add = 1;
    } else if (c >= 160 && c <= 167) {
        add = 1;
    } else if (c == 225) {
        add = 1;
    } else if (data.allow_punctuation) {
        if (c == ',' || c == '.' || c == '?' || c == '!') {
            add = 1;
        }
    }
    if (add) {
        add_char(c);
    }
}
