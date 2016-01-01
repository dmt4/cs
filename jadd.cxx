#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <systemd/sd-bus.h>

// from Lennart's blog
int main(int argc, char *argv[]) {
        
        if (argc != 3) {
            fprintf(stderr, "usage:\n  %s name request\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        std::string jname(argv[1]);
        std::string jreq(argv[2]);
        
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;
        sd_bus *bus = NULL;
        uint64_t jid = 0;
        int r;

        /* Connect to the system bus */
        r = sd_bus_open_system(&bus);
        if (r < 0) {
                fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
                goto finish;
        }

        /* Issue the method call and store the respons message in m */
        r = sd_bus_call_method(bus,
                               "org.cs",           /* service to contact */
                               "/org/cs",          /* object path */
                               "org.cs.j",   /* interface name */
                               "jadd",                          /* method name */
                               &error,                               /* object to return error in */
                               &m,                                   /* return message on success */
                               "as",                               /* input signature */
                               2,
                               jname.c_str(),                            /* first argument */
                               jreq.c_str());                           /* second argument */
        if (r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                goto finish;
        }

        /* Parse the response message */
        r = sd_bus_message_read(m, "t", &jid);
        if (r < 0) {
                fprintf(stderr, "Failed to parse response message: %s\n", strerror(-r));
                goto finish;
        }

        std::cout << jid << '\n';

finish:
        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        sd_bus_unref(bus);

        return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}