#include <dirent.h>
#include <sys/types.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <algorithm>
#include <cinttypes>
#include <string>
#include <vector>

#include "util.h"

using std::uint64_t;
using std::uint32_t;
using std::string;
using std::vector;

uint64_t get_timestamp() {
    struct timespec ts;

/*
 *https://gist.github.com/jbenet/1087739
 */
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec  = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    // assert(_POSIX_C_SOURCE >= 199309L);
    clock_gettime(CLOCK_REALTIME, &ts);
#endif

    return 1000000000L * ts.tv_sec + ts.tv_nsec;
}

static const char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static const char decoding_table[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

static const int mod_table[] = {0, 2, 1};

string base64_encode(const string &data) {
    size_t input_length  = data.size();
    size_t output_length = 4 * ((input_length + 2) / 3);
    string encoded_data;
    encoded_data.resize(output_length);
    for(size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for(int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    for(size_t i = 0; i < encoded_data.size(); ++i)
        if(encoded_data[i] == '\0') {
            encoded_data.resize(i);
            break;
        }

    return encoded_data;
}

string base64_decode(const string &data) {
    size_t input_length = data.size();

    if(input_length % 4 != 0)
        return "";

    size_t length = input_length / 4 * 3;
    if(data[input_length - 1] == '=')
        --length;
    if(data[input_length - 2] == '=')
        --length;

    string decoded_data;
    decoded_data.resize(length);

    for(size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_b =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_c =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_d =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) +
                          (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if(j < length)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if(j < length)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if(j < length)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    for(size_t i = 0; i < decoded_data.size(); ++i)
        if(decoded_data[i] == '\0') {
            decoded_data.resize(i);
            break;
        }

    return decoded_data;
}

vector<string> get_file_list(const char *const base_directory) {
    DIR *the_directory = opendir(base_directory);

    vector<string> result;
    if(!the_directory)
        return result;


    struct dirent *curr_file = NULL;
    while((curr_file = readdir(the_directory))) {
        if(curr_file->d_type == DT_DIR)
            ;  // TODO..
        else
            result.emplace_back(curr_file->d_name);
    }

    std::sort(result.begin(), result.end());

    return result;
}


string str_implode(const vector<string> &svec, char seperator) {
    if(svec.empty())
        return "";

    size_t len = 0;
    for(const string &s : svec)
        len += s.size();
    string result;
    result.reserve(len + svec.size());
    for(size_t i = 0; i < svec.size(); ++i) {
        result += svec[i];
        result.push_back(seperator);
    }
    result += svec.back();
    return result;
}