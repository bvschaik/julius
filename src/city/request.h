#ifndef CITY_REQUEST_H
#define CITY_REQUEST_H

enum {
    CITY_REQUEST_STATUS_NOT_ENOUGH_RESOURCES = -1,
    CITY_REQUEST_STATUS_CONFIRM_SEND_LEGIONS = -2,
    CITY_REQUEST_STATUS_NO_LEGIONS_SELECTED = -3,
    CITY_REQUEST_STATUS_NO_LEGIONS_AVAILABLE = -4,
};

#define CITY_REQUEST_MAX_ACTIVE 5

int city_request_has_troop_request(void);
int city_request_get_status(int index);

#endif // CITY_REQUEST_H
