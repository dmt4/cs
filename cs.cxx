#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cfloat>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <iomanip>
#include <limits>

#include <iterator>
#include <list>
#include <vector>
#include <map>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <valarray>

#include <systemd/sd-bus.h>


using std::vector;
using std::string;
using std::list;
using std::map;
using std::ostream;
using std::setw;


// this is defined in another file due to syntactic incompatibility of c11 with c++
extern "C" const sd_bus_vtable job_ops_vtable[];


class user_creds_t {
public:
    user_creds_t(uid_t uid, gid_t gid, int ngids, gid_t *gids): 
        uid(uid), gid(gid), gids(gids, gids+ngids) {}
    
    user_creds_t(sd_bus_creds *creds):
        uid(99), gid(99) {
        sd_bus_creds_get_euid(creds, &uid);
        sd_bus_creds_get_egid(creds, &gid);
                
        const gid_t *gids = NULL;
        int ngids = sd_bus_creds_get_supplementary_gids(creds, &gids);
        this->gids = vector<gid_t>(gids, gids+ngids);
    }
    
    uid_t uid;
    gid_t gid;
    vector<gid_t> gids;
    
    friend ostream& operator<< (ostream &, const user_creds_t &);
    
//    later capabilities, connections etc...
};

ostream& operator<< (ostream &o, const user_creds_t &uc);

class job_t {
public:
    uint64_t id;
    const std::valarray<string> status_s {"queued", "running", "past"};
    enum status_t {queued, running, past};
    user_creds_t creds; 
    
    job_t(string &name, string &req, user_creds_t &creds): 
        name(name), req(req), creds(creds), id(++id0), status(queued) {}
    job_t(const char *name, const char *req, sd_bus_creds *creds): 
        name(string(name)), req(string(req)), creds(user_creds_t(creds)), id(++id0), status(queued) {}
    
    ~job_t() {
        std::cerr << "deleted: " << *this << '\n';
    }
    
    
    friend ostream& operator<< (ostream &, const job_t &);
    
private:
    static uint64_t id0;
    string name;
    string req;
    vector<string> nodes; // list of addresses
    status_t status;
};

uint64_t job_t::id0 = 0;



ostream& operator<< (ostream &o, const job_t &j);


// template<class T> std::ostream& operator<<(std::ostream& o, const T& v) {
//     std::copy(v.begin(), v.end(), std::ostream_iterator<typename T::value_type>(o, " "));
//     return o;
// }

template<class T> std::ostream& operator<<(std::ostream& o, const std::vector<T>& v) {
    o << '{'; std::copy(v.begin(), v.end(), std::ostream_iterator<T>(o, " ")); o << '}';
    return o;
}

template<class T> std::ostream& operator<<(std::ostream& o, const std::list<T>& v) {
    o << '{'; std::copy(v.begin(), v.end(), std::ostream_iterator<T>(o, " ")); o << '}';
    return o;
}

template<typename T, typename S>
std::ostream& operator<< (std::ostream & o, const std::unordered_map<T,S>& v) {
    o << '{'; for (auto& i : v) o << '{' << i.first << ": " << i.second << "} "; o << '}';
    return o;
}


template<typename T, typename S>
std::ostream& operator<< (std::ostream & o, const std::map<T,S>& v) {
    for (auto& i : v) o << i.first << ": " << i.second << '\n';
    return o;
}



ostream& operator<< (ostream &o, const user_creds_t &uc) {
    o << "{uid:" << uc.uid 
      << " gid:" << uc.gid 
      << " gids:" << uc.gids
      << "}";
    return o;
}



ostream& operator<< (ostream &o, const job_t &j) {
    
    o << "{id: "     << setw(8) << j.id   
      << " name: "   << setw(8) << j.name 
      << " req: "    << setw(8) << j.req  
      << " creds: "             << j.creds
//       << " uid: "    << setw(8) << j.creds.uid
      << " status: " << setw(8) << j.status_s[j.status]
      << "}";
    return o;
}



using jid_t = uint64_t;
using jobs_t = map<jid_t,job_t>;
using jdel_lst_t = map<jid_t,string>;

std::ostream& operator<< (std::ostream & o, const jobs_t& js) {
    for (auto& i : js) o << i.second << '\n';
    return o;
}

std::ostream& operator<< (std::ostream & o, const jdel_lst_t& js) {
    for (auto& i : js) o << i.first << ": " << i.second << '\n';
    return o;
}




extern "C" int jadd(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
//     most of this is here rather than in the constructors of job_t etc.. because it is easy to return the proper errors to the bus.
        sd_bus_creds *creds = sd_bus_message_get_creds(m);
        uint64_t creds_mask = sd_bus_creds_get_mask(creds);
        if (!(
            (creds_mask & SD_BUS_CREDS_EUID  ) &&
            (creds_mask & SD_BUS_CREDS_EGID  ) &&
            (creds_mask & SD_BUS_CREDS_SUPPLEMENTARY_GIDS)         
        )) {
            fprintf(stderr, "uid, gid or supplementary gids of caller not available\n");
            return -ENODATA;
        }
        
        const char *name, *req;
        int r = sd_bus_message_read(m, "as", 2, &name, &req);
        if (r < 0) {
                fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
                return r;
        }
        
//         job_t j(string(name), string(req), user_creds_t(creds)); //this gets into some mess
        job_t j(name, req, creds);
        
        
        jobs_t& jobs = *(static_cast<jobs_t*>(userdata));
        jobs.emplace(j.id, j);  //try_emplace not in gcc yet (v5.3.0)
        
        return sd_bus_reply_method_return(m, "t", j.id);
}

extern "C" int jdel(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
        sd_bus_creds *creds = sd_bus_message_get_creds(m);
        uid_t uid(99); sd_bus_creds_get_euid(creds, &uid);
        
        jid_t *ids;
        size_t sz;
        
//      the signature is really "at"
        int r = sd_bus_message_read_array(m, 't', (const void**) &ids, &sz);
        if (r < 0) {
            fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
            return r;
        }
        
        jobs_t& jobs = *(static_cast<jobs_t*>(userdata));
        
        jdel_lst_t res;
        for (jid_t *id = ids; id < ids+sz/sizeof(jid_t*); ++id) {
            res.emplace(*id, "pending"); 
        }
   
        for (auto& jp: res) {
            auto itr = jobs.find(jp.first);
            if (itr != jobs.end()) {
                if (itr->second.creds.uid == uid || uid == 0) {
                    jobs.erase(itr);
                    jp.second = "done";
                } else {
                    jp.second = "access denied";
                }
            } else {
                jp.second = "nonexistent";
            }            
        }
        
        std::ostringstream s;
        s << std::left << '\n' << res;
        return sd_bus_reply_method_return(m, "s", s.str().c_str());
}



extern "C" int jlst(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
        jobs_t& jobs = *(static_cast<jobs_t*>(userdata));
        
        std::ostringstream s;
        s << std::left << '\n' << jobs;
        return sd_bus_reply_method_return(m, "s", s.str().c_str());
}

// from Lennart's blog
int main(int argc, char *argv[]) {
    sd_bus *bus = NULL;
    sd_bus_slot *slot = NULL;
    int r;
    
//     use map for now as it is sorted but may switch to unordered_map later and sort on print only
    jobs_t jobs;

//         r = sd_bus_open_user(&bus);
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to bus: %s\n", strerror(-r));
        goto finish;
    }


//         will not set creds without this.
    r = sd_bus_negotiate_creds(bus, 1, _SD_BUS_CREDS_ALL);
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