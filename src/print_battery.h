#ifndef  __swaystatus_print_battery_H__
# define __swaystatus_print_battery_H__

struct Print_Battery_Data {
    void *upclient;
};

void init_print_battery(struct Print_Battery_Data *data);

void print_battery(const struct Print_Battery_Data *data);

#endif
