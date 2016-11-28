#ifndef __NUTIL_H__
#define __NUTIL_H__
// a C++ implimentation of the old deps/util.h
#include <cinttypes>
#include <iostream>
#include <string>
#include <vector>
using std::uint64_t;
using std::string;
using std::vector;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;

// macro for easy debugging
#ifdef DEBUG
#define PERROR(x)          \
    do {                   \
        cerr << x << endl; \
    } while(false);
#else
#define PERROR(x)
#endif

uint64_t get_timestamp();
string base64_encode(const string& data);
string base64_decode(const string& data);
vector<string> get_file_list(const char* const base_directory);


#endif