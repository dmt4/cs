#ifndef UTIL_H
#define UTIL_H

#include <limits>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
#include <valarray>
#include <array>
#include <vector>
#include <list>
#include <unordered_set>
#include <map>
#include <unordered_map>



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

#endif // UTIL_H