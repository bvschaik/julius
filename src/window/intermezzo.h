#ifndef WINDOW_INTERMEZZO_H
#define WINDOW_INTERMEZZO_H

typedef enum {
    INTERMEZZO_MISSION_BRIEFING = 0,
    INTERMEZZO_FIRED = 1,
    INTERMEZZO_WON = 2,
} intermezzo_type;

void window_intermezzo_show(intermezzo_type type, void (*callback)(void));

#endif // WINDOW_INTERMEZZO_H
