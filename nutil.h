#ifndef __NUTIL_H__
#define __NUTIL_H__
// a C++ implimentation of the old deps/util.h
#include <cinttypes>
#include <string>
using std::uint64_t;
using std::string;

uint64_t get_timestamp();
string base64_encode(const string& data);
string base64_decode(const string& data);


#endif