#ifndef DATA_EVENT_H
#define DATA_EVENT_H

#define MAX_EVENTS 20

enum
{
    SpecialEvent_NotStarted = 0,
    SpecialEvent_InProgress = 1,
    SpecialEvent_Finished = 2
};

extern struct _Data_Event
{
    short lastInternalInvasionId;
    int timeLimitMaxGameYear;
    struct
    {
        int gameYear;
        int month;
        int state;
    } emperorChange;
    struct
    {
        int gameYear;
        int month;
        int endMonth;
        int state;
    } gladiatorRevolt;
} Data_Event;

#endif
