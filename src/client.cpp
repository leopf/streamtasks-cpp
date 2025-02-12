#include <streamtasks.hpp>

void readexact(int __fd, void *__buf, size_t __nbytes) 
{
    int offset = 0;
    while (offset < __nbytes) 
    {
        offset += read(__fd, (char*)__buf + offset, __nbytes - offset);
    }
}

StreamtasksClient::StreamtasksClient(int socketfd)
{
    this->sockfd = socketfd;
}

StreamtasksClient::~StreamtasksClient()
{
    close(sockfd);
}

uint64_t StreamtasksClient::get_address()
{
    return address;
}

void StreamtasksClient::send_str(const std::string data)
{
    size_t l = data.length();
    uint8_t l_data[4] = {(uint8_t)((l >> 0) & 0xff), (uint8_t)((l >> 8) & 0xff), (uint8_t)((l >> 16) & 0xff), (uint8_t)((l >> 24) & 0xff)};
    write(sockfd, CONNECTION_SYNC_WORD, 4);
    write(sockfd, l_data, 4);
    write(sockfd, data.data(), l);
}

std::string StreamtasksClient::recv_str()
{
    int sync_index = 0;
    uint8_t sync_data[1];
    while (sync_index < 4)
    {
        readexact(sockfd, &sync_data, 1);
        if (sync_data[0] != CONNECTION_SYNC_WORD[sync_index])
        {
            sync_index = 0;
        }
        else
        {
            sync_index++;
        }
    }

    uint8_t raw_data_len[4];
    readexact(sockfd, &raw_data_len, 4);
    uint32_t data_len = raw_data_len[0] | (raw_data_len[1] << 8) | (raw_data_len[2] << 16) | (raw_data_len[3] << 24);
    printf("data len: %lu\n", data_len);
    std::string out_str(data_len, '\0');
    readexact(sockfd, out_str.data(), data_len);
    return out_str;
}

StreamtasksMessage StreamtasksClient::recv_message()
{
    std::string data = recv_str();
    std::string err;
    MsgPack content = MsgPack::parse(data, err);
    if (err.length() > 0)
    {
        throw std::runtime_error(err);
    }
    return StreamtasksMessage(content);
}

MsgPack StreamtasksClient::init(MsgPack handshake_data)
{
    send_str(handshake_data.dump());

    std::string err;
    MsgPack other_data = MsgPack::parse(recv_str(), err);
    if (err.length() > 0)
    {
        throw std::runtime_error(err);
    }

    return other_data;
}

StreamtasksClient StreamtasksClient::connect_unix(const char *socket_path)
{
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        throw std::runtime_error("Failed to open socket file");
    }
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        close(sockfd);
        throw std::runtime_error("Failed to connect to socket.");
    }

    return StreamtasksClient(sockfd);
}
void StreamtasksClient::send_message(StreamtasksMessage message)
{
    send_str(message.serialize());
}

void StreamtasksClient::send_signal(uint64_t address, uint64_t port, const char* descriptor, MsgPack body)
{
    send_message(StreamtasksMessage::addressed_message(address, port, MsgPack::object 
    {
        {"descriptor", descriptor}, 
        {"body", body }
    }));
}

MsgPack StreamtasksClient::send_fetch(uint64_t address, uint64_t port, const char *descriptor, MsgPack body)
{
    if (this->address == 0) 
    {
        throw std::runtime_error("Missing address!");
    }

    std::random_device rand_dev;
    unsigned int return_port = rand_dev();

    send_message(StreamtasksMessage::addressed_message(address, port, MsgPack::object 
    {
        {"descriptor", descriptor}, 
        {"body", body },
        {"return_port", return_port },
        {"return_address", this->address },
    }));

    while (1)
    {
        StreamtasksMessage message = recv_message();
        if (message.id() == StreamtasksMessageID::MSG_ID_ADDRESSED && message.port() == return_port) 
        {
            MsgPack data = message.data();
            if (data.is_object()) {
                if (data["error"] == true) 
                {
                    throw std::runtime_error("Fetch error!");
                }
                else 
                {
                    return data["body"];
                }
            }
        }
    }
}

uint64_t StreamtasksClient::resolve_address(const char *address)
{
    MsgPack response = send_fetch(0, StreamtasksPort::PORT_FETCH, "resolve_address_name", MsgPack::object {
        { "address_name", address }
    });
    if (response.is_object() && response["address"].is_int()) 
    {
        return response["address"].uint64_value();
    }
    throw std::runtime_error("invalid resolve response!");
}

void StreamtasksClient::request_address()
{
    send_message(StreamtasksMessage::in_topics_changed_message({1}, {}));

    std::random_device rand_dev;
    unsigned int request_id = rand_dev();

    send_signal(0, PORT_SIGNAL, "request_addresses", MsgPack::object {
        {
            {"request_id", request_id},
            {"count", 1},
        }
    });

    while (1)
    {
        StreamtasksMessage message = recv_message();
        if (message.id() == StreamtasksMessageID::MSG_ID_TOPIC_DATA && message.topic() == 1) 
        {
            MsgPack data = message.data();
            if (data.is_object()) 
            {
                MsgPack::object items = data.object_items();
                if (items.count("request_id") > 0 && items.count("addresses") > 0 && items["request_id"].uint32_value() == request_id) 
                {
                    for (MsgPack address_w : items["addresses"].array_items()) 
                    {
                        address = address_w.uint64_value();
                    }
                    if (address == 0) 
                    {
                        throw std::runtime_error("failed to request address!");
                    }
                    break;
                }
            }
        }
    }

    send_message(StreamtasksMessage::addresses_changed_message({address}, {}));
    send_message(StreamtasksMessage::in_topics_changed_message({}, {1}));
}
