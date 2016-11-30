#ifndef __NUTIL_H__
#define __NUTIL_H__
// a C++ implimentation of the old deps/util.h
#include <cinttypes>
#include <iostream>
#include <sstream>
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
    } while(false)
#else
#define PERROR(x)
#endif

enum COMMAND_TYPES {
    C_NONE                 = 0,
    C_GET_REMOTE_FILE_LIST = 65,
    C_RESPONSE_REMOTE_FILE_LIST,
    C_OPEN_FILE_REQUEST,
    C_RESPONSE_FILE_INFO,
    C_OTHER = 122,
};

uint64_t get_timestamp();
string base64_encode(const string& data);
string base64_decode(const string& data);
vector<string> get_file_list(const char* const base_directory);

string str_implode(const vector<string>& svec, char seperator = '&');


#endif