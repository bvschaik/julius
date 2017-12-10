#include "loki/loki.h"

#include "input/keyboard.h"
int string_length(const uint8_t *str)
{
    int length = 0;
    while (*str) {
        length++;
        str++;
    }
    return length;
}

CREATE_MOCK2(int, Widget_Text_getWidth, const uint8_t*, font_t)

INIT_MOCKS(
    INIT_MOCK(Widget_Text_getWidth)
)

void test_keyboard_capture()
{
    uint8_t text[100] = "help";
    
    keyboard_start_capture(text, 100, 1, 100, 0);
    keyboard_character('a');
    
    assert_eq_string("helpa", (char*) text);
}

void test_keyboard_delete()
{
    uint8_t text[100] = "help";
    
    keyboard_start_capture(text, 100, 1, 100, 0);
    
    keyboard_delete();
    assert_eq_string("help", (char*) text);
    
    keyboard_left();
    keyboard_delete();
    assert_eq_string("hel", (char*) text);
    
    keyboard_home();
    keyboard_delete();
    assert_eq_string("el", (char*) text);
}

RUN_TESTS(input.keyboad,
    ADD_TEST(test_keyboard_capture)
    ADD_TEST(test_keyboard_delete)
)
