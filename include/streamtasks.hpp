#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>
#include <msgpack11.hpp>
#include <random>
#include <stdio.h>

using msgpack11::MsgPack;

const unsigned char CONNECTION_SYNC_WORD[4] = {0xb8, 0x23, 0xa0, 0x6f};

enum StreamtasksMessageID {
    MSG_ID_TOPIC_DATA = 0,
    MSG_ID_TOPIC_CONTROL = 1,
    MSG_ID_ADDRESSED = 2,
    MSG_ID_ADDRESSES_CHANGED = 3,
    MSG_ID_IN_TOPICS_CHANGED = 4,
    MSG_ID_OUT_TOPICS_CHANGED = 5,
};

enum StreamtasksPort {
    PORT_FETCH = 100,
    PORT_SIGNAL = 103,
};

struct TopicControlData
{
    bool paused;
};


class StreamtasksMessage
{
protected:

public:
    MsgPack content;
    StreamtasksMessage(MsgPack content);
    MsgPack data();
    uint64_t topic();
    bool paused();
    uint64_t address();
    uint64_t port();
    std::string serialize();
    StreamtasksMessageID id();

    void print();

    static StreamtasksMessage topic_data_message(uint64_t topic, MsgPack data);
    static StreamtasksMessage topic_control_message(uint64_t topic, TopicControlData control);
    static StreamtasksMessage addressed_message(uint64_t address, uint64_t port, MsgPack data);
    static StreamtasksMessage addresses_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove); // cost=0
    static StreamtasksMessage in_topics_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove);
    static StreamtasksMessage out_topics_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove); ; // cost=0
};

class StreamtasksClient
{
private:
    int sockfd;
    uint64_t address = 0;

    void send_str(const std::string data);
    std::string recv_str();

public:
    StreamtasksClient(int sockfd);
    ~StreamtasksClient();

    uint64_t get_address();

    MsgPack init(MsgPack handshake_data);
    StreamtasksMessage recv_message();
    void send_message(StreamtasksMessage message);
    void send_signal(uint64_t address, uint64_t port, const char* descriptor, MsgPack body);
    MsgPack send_fetch(uint64_t address, uint64_t port, const char* descriptor, MsgPack body);

    uint64_t resolve_address(const char* address);
    void request_address();

    static StreamtasksClient connect_unix(const char *socket_path);
};
