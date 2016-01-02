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

// these are non-const in systemd/sd-bus.h but shall really be const!
int sd_bus_creds_get_euid(const sd_bus_creds *c, uid_t *euid);
int sd_bus_creds_get_egid(const sd_bus_creds *c, gid_t *egid);
int sd_bus_creds_get_supplementary_gids(const sd_bus_creds *c, const gid_t **gids);




using std::vector;
using std::string;
using std::list;
using std::map;
using std::ostream;
using std::setw;



#ifndef JOBS_H
#define JOBS_H

using jid_t = uint64_t;
using jdel_lst_t = std::unordered_map<jid_t,string>;


class user_creds_t {
public:
    user_creds_t(const uid_t uid, const gid_t gid, const int ngids, const gid_t *gids): 
        uid(uid), gid(gid), gids(gids, gids+ngids) {}
    
    user_creds_t(const sd_bus_creds *creds);
    
    uid_t uid;
    gid_t gid;
    vector<gid_t> gids;
    
    friend ostream& operator<< (ostream &, const user_creds_t &);
    
//    later capabilities, connections etc...
};

ostream& operator<< (ostream &o, const user_creds_t &uc);

class job_t {
public:
    jid_t id;
    const std::valarray<string> status_s {"queued", "running", "past"};
    enum status_t {queued, running, past};
    user_creds_t creds; 
    
    job_t(const string &name, const string &req, const user_creds_t &creds, const jid_t id = 0): 
        name(name), req(req), creds(creds), id(id), status(queued) {}
    job_t(const char *name, const char *req, const sd_bus_creds *creds, const jid_t id = 0): 
        name(string(name)), req(string(req)), creds(user_creds_t(creds)), id(id), status(queued) {}
    
    ~job_t() {
        std::cerr << "deleted: " << *this << '\n';
    }
    
    
    friend ostream& operator<< (ostream &, const job_t &);
    
private:
    string name;
    string req;
    vector<string> nodes; // list of addresses
    status_t status;
};



ostream& operator<< (ostream &o, const job_t &j);


class jobs_t {
    jid_t id0;
    map<jid_t,job_t> js;
    
public:    
    jobs_t(const jid_t id0 = 0): id0(id0) {}
    
    jid_t add(const job_t &j);
    jid_t add(const char *name, const char *req, const sd_bus_creds *creds);
    
    string del(const vector<jid_t>& jids, const user_creds_t &creds);
    string del(const int njids, const jid_t *jids, const sd_bus_creds *creds);
    
    string lst();
};

#endif // JOBS_H