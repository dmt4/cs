#include "jobs.hxx"

int sd_bus_creds_get_euid(const sd_bus_creds *c, uid_t *euid) {
    return sd_bus_creds_get_euid(const_cast<sd_bus_creds*>(c), euid);
}
int sd_bus_creds_get_egid(const sd_bus_creds *c, gid_t *egid) {
    return sd_bus_creds_get_egid(const_cast<sd_bus_creds*>(c), egid);
}
int sd_bus_creds_get_supplementary_gids(const sd_bus_creds *c, const gid_t **gids) {
    return sd_bus_creds_get_supplementary_gids(const_cast<sd_bus_creds*>(c), gids);
}


user_creds_t::user_creds_t(const sd_bus_creds *creds):
    uid(99), gid(99) {
    sd_bus_creds_get_euid(creds, &uid);
    sd_bus_creds_get_egid(creds, &gid);
            
    const gid_t *gids = NULL;
    int ngids = sd_bus_creds_get_supplementary_gids(creds, &gids);
    this->gids = vector<gid_t>(gids, gids+ngids);
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

// std::ostream& operator<< (std::ostream & o, const jobs_t& js) {
//     for (auto& i : js) o << i.second << '\n';
//     return o;
// }


std::ostream& operator<< (std::ostream & o, const jdel_lst_t& js) {
    for (auto& i : js) o << i.first << ": " << i.second << '\n';
    return o;
}


jid_t jobs_t::add(const job_t &j) {
    jid_t jid(++id0);
    auto jn = js.emplace(jid, j); //try_emplace not in gcc yet (v5.3.0)
    if (jn.second) jn.first->second.id = jid;
    return jid;
}

jid_t jobs_t::add(const char *name, const char *req, const sd_bus_creds *creds) {
    job_t j(name, req, creds);
    return add(j);
}

string jobs_t::del(const vector<jid_t>& jids, const user_creds_t &creds) {
    
    jdel_lst_t res;
    for (auto& jid: jids) res.emplace(jid, "pending");

    for (auto& jp: res) {
        auto itr = js.find(jp.first);
        if (itr != js.end()) {
            if (itr->second.creds.uid == creds.uid || 0 == creds.uid) {
                js.erase(itr);
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
    return s.str();
}

string jobs_t::del(const int njids, const jid_t *jids, const sd_bus_creds *creds) {
    return del(vector<jid_t>(jids, jids+njids), user_creds_t(creds));
}

string jobs_t::lst() {
    std::ostringstream s;
    s << std::left << '\n' << js;
    return s.str();
}
