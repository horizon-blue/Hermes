#ifndef __NSERVER_H__
#define __NSERVER_H__

#include <string>
using std::string;

class Client {
public:
    Client() = default;

    string get_filename() const { return filename; }

private:
    int socket;
    string filename;
};


#endif