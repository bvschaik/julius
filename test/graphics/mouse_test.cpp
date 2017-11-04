#include "loki/loki.h"

#include "graphics/mouse.hpp"

NO_MOCKS()

void test_mouse_set_position()
{
    mouse_set_position(123, 456);

    assert_eq(123, mouse_get()->x);
    assert_eq(456, mouse_get()->y);
}

void test_mouse_set_inside_window()
{
    mouse_set_inside_window(1);
    assert_true(mouse_get()->is_inside_window);

    mouse_set_inside_window(0);
    assert_false(mouse_get()->is_inside_window);
}

void test_mouse_set_scroll()
{
    mouse_set_scroll(SCROLL_DOWN);
    assert_eq(SCROLL_DOWN, mouse_get()->scrolled);
}

void test_mouse_down()
{
    assert_false(mouse_get()->left.new_is_down);
    mouse_set_left_down(1);
    assert_true(mouse_get()->left.new_is_down);

    assert_false(mouse_get()->right.new_is_down);
    mouse_set_right_down(1);
    assert_true(mouse_get()->right.new_is_down);
}

void test_mouse_determine_state_down()
{
    // reset state
    mouse_set_left_down(0);
    mouse_set_right_down(0);
    mouse_reset_up_state();

    mouse_set_left_down(1);
    mouse_determine_button_state();

    assert_true(mouse_get()->left.went_down);
    assert_true(mouse_get()->left.is_down);
    assert_false(mouse_get()->right.went_down);
    assert_false(mouse_get()->right.is_down);
}

void test_mouse_determine_state_up()
{
    // reset state
    mouse_set_left_down(0);
    mouse_set_right_down(0);
    mouse_reset_up_state();

    mouse_set_right_down(1);
    mouse_determine_button_state();

    assert_true(mouse_get()->right.is_down);

    mouse_set_right_down(0);
    mouse_determine_button_state();

    assert_false(mouse_get()->left.went_up);
    assert_true(mouse_get()->right.went_up);
    assert_false(mouse_get()->right.is_down);
}

RUN_TESTS(graphics.mouse,
          ADD_TEST(test_mouse_set_position)
          ADD_TEST(test_mouse_set_inside_window)
          ADD_TEST(test_mouse_set_scroll)
          ADD_TEST(test_mouse_down)
          ADD_TEST(test_mouse_determine_state_down)
          ADD_TEST(test_mouse_determine_state_up)
         )
