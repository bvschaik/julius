#ifndef WINDOW_EDITOR_CUSTOM_VARIABLES_H
#define WINDOW_EDITOR_CUSTOM_VARIABLES_H

#include "scenario/data.h"

void window_editor_custom_variables_show(int from_editor);

void window_editor_custom_variables_select_show(void (*callback)(custom_variable_t *));

#endif // WINDOW_EDITOR_CUSTOM_VARIABLES_H
