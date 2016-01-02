#include "jobs.hxx"


// this is defined in another file due to syntactic incompatibility of c11 with c++
extern "C" const sd_bus_vtable job_ops_vtable[];


extern "C" int jadd(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
//     most of this is here rather than in the constructors of job_t etc.. because it is easy to return the proper errors to the bus.
    auto* creds = sd_bus_message_get_creds(m);
    auto creds_mask = sd_bus_creds_get_mask(creds);
    if (!((creds_mask & SD_BUS_CREDS_EUID) && (creds_mask & SD_BUS_CREDS_EGID)
        && (creds_mask & SD_BUS_CREDS_SUPPLEMENTARY_GIDS))) {
        fprintf(stderr, "uid, gid or supplementary gids of caller not available\n");
        return -ENODATA;
    }
    
    const char *name, *req;
    
    auto r = sd_bus_message_read(m, "as", 2, &name, &req);
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }
    
    auto& jobs = *(static_cast<jobs_t*>(userdata));
    return sd_bus_reply_method_return(m, "t", jobs.add(name, req, creds));
}

extern "C" int jdel(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    auto* creds = sd_bus_message_get_creds(m);
    auto creds_mask = sd_bus_creds_get_mask(creds);
    if (!((creds_mask & SD_BUS_CREDS_EUID) && (creds_mask & SD_BUS_CREDS_EGID)
        && (creds_mask & SD_BUS_CREDS_SUPPLEMENTARY_GIDS))) {
        fprintf(stderr, "uid, gid or supplementary gids of caller not available\n");
        return -ENODATA;
    }
    
    jid_t *jids;
    size_t sz;
    
    auto r = sd_bus_message_read_array(m, 't', (const void**) &jids, &sz); // the signature is really "at"
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }
    
    auto& jobs = *(static_cast<jobs_t*>(userdata));
    return sd_bus_reply_method_return(m, "s", jobs.del(sz/sizeof(jid_t*), jids, creds).c_str());
}



extern "C" int jlst(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
        auto& jobs = *(static_cast<jobs_t*>(userdata));
        return sd_bus_reply_method_return(m, "s", jobs.lst().c_str());
}

// from Lennart's blog
int main(int argc, char *argv[]) {
    sd_bus *bus = NULL;
    sd_bus_slot *slot = NULL;
    int r;
    
    jobs_t jobs;

//         r = sd_bus_open_user(&bus);
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to bus: %s\n", strerror(-r));
        goto finish;
    }

//         will not set creds without this.
//     r = sd_bus_negotiate_creds(bus, 1, _SD_BUS_CREDS_ALL);
    r = sd_bus_negotiate_creds(bus, 1, SD_BUS_CREDS_EUID|SD_BUS_CREDS_EGID|SD_BUS_CREDS_SUPPLEMENTARY_GIDS);
    if (r < 0) {
        fprintf(stderr, "Failed to negotiate creds: %s\n", strerror(-r));
        goto finish;
    }
    
    r = sd_bus_add_object_vtable(bus,
                                &slot,
                                "/org/cs",  /* object path */
                                "org.cs.j",   /* interface name */
                                job_ops_vtable,
                                &jobs);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_request_name(bus, "org.cs", 0);
    if (r < 0) {
        fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-r));
        goto finish;
    }
    
    for (;;) {
        /* Process requests */
        r = sd_bus_process(bus, NULL);
        
        if (r < 0) {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-r));
            goto finish;
        }
        
        /* we processed a request, try to process another one, right-away */
        if (r > 0) continue;

        /* Wait for the next request to process */
        r = sd_bus_wait(bus, (uint64_t) -1);
        if (r < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-r));
            goto finish;
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

