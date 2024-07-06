#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>
#include <msgpack11.hpp>

using msgpack11::MsgPack;

const unsigned char CONNECTION_SYNC_WORD[4] = {0xb8, 0x23, 0xa0, 0x6f};

enum StreamtasksMessageType {

};

class StreamtasksMessage
{
protected:
    MsgPack content;

public:
    StreamtasksMessage(MsgPack content);
    std::vector<uint8_t> data();
    unsigned long topic();
    bool paused();
    unsigned long address();
    unsigned long port();

    void print();
};

class StreamtasksClient
{
private:
    int sockfd;

    void send_str(const std::string data);
    std::string recv_str();

public:
    StreamtasksClient(int sockfd);
    ~StreamtasksClient();

    MsgPack init(MsgPack handshake_data);
    StreamtasksMessage recv_message();

    static StreamtasksClient connect_unix(const char *socket_path);
};
