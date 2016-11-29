#include <arpa/inet.h>
#include <netdb.h>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <string>
#include "nutil.h"
using namespace std;

int main() {
    cout << "Enter a string: " << endl;
    string temp;
    getline(cin, temp);
    cout << "Original string: " << temp << endl;
    string result = base64_encode(temp);
    cout << "Encrypted string: " << result << endl;
    result = base64_decode(result);
    cout << "Decrypted string: " << result << endl;
    if(temp == result)
        cout << "Strings match." << endl;
    else
        cout << "Strings does not match." << endl;
    // int32_t len = 10;
    // len         = htonl(len);

    // char buf[4];
    // memcpy(buf, reinterpret_cast<char*>(&len), 4);
    // int32_t len_2;
    // memcpy(reinterpret_cast<char*>(&len_2), buf, 4);
    // len_2 = ntohl(len_2);

    // cout << len_2 << endl;

    return 0;
}