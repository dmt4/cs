// this is split because the SD_BUS_* defs use "." initialisers form c11 and they are illegal in c++.

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>

int jadd(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
int jlst(sd_bus_message *m, void *userdata, sd_bus_error *ret_error); 
int jdel(sd_bus_message *m, void *userdata, sd_bus_error *ret_error); 

const sd_bus_vtable job_ops_vtable[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_METHOD("jadd", "as", "t", jadd, SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_METHOD("jdel", "at", "s", jdel, SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_METHOD("jlst",   "", "s", jlst, SD_BUS_VTABLE_UNPRIVILEGED), // "as"
        SD_BUS_VTABLE_END
};

