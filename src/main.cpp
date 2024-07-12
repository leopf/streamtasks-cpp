#include <stdio.h>
#include "streamtasks.hpp"
#include <stdexcept>
#include <msgpack11debug.hpp>


int main(int argc, char const *argv[])
{
    if (argc < 2) {
        throw std::runtime_error("You must specify a node name argument!");
    }

    char buff[100];
    snprintf(buff, sizeof(buff), "/tmp/streamtasks-%s.sock", argv[1]);
    printf(buff);
    printf("\n");
    StreamtasksClient client = StreamtasksClient::connect_unix(buff);
    auto handshake_data = client.init(MsgPack::object { { "version", "0.1" } });
    
    printf("System version: ");
    printf(handshake_data["version"].string_value().c_str());
    printf("\n");

    client.request_address();
    printf("address: %d\n", client.get_address());

    uint64_t tm_address = client.resolve_address("task_manager");
    printf("tm address: %d\n", tm_address);

    MsgPack task_hosts = client.send_fetch(tm_address, StreamtasksPort::PORT_FETCH, "list_task_hosts", MsgPack());
    printf("got hosts\n");
    show(task_hosts);

    while (1)
    {
        auto message = client.recv_message();
        message.print();
    }
    
    return 0;
}
