#ifndef DATA_TUTORIAL_H
#define DATA_TUTORIAL_H

struct _Data_Tutorial
{
    struct
    {
        int fire;
        int crime;
        int collapse;
        int senateBuilt;
    } tutorial1;
    struct
    {
        int granaryBuilt;
        int population250Reached;
        int population450Reached;
        int potteryMade;
        int potteryMadeYear;
    } tutorial2;
    struct
    {
        int disease;
    } tutorial3;
};

extern _Data_Tutorial Data_Tutorial;

#endif
