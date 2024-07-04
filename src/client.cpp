#include <streamtasks.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>

StreamtasksClient::StreamtasksClient(int socketfd)
{
    this->sockfd = socketfd;
}

StreamtasksClient::~StreamtasksClient()
{
    close(sockfd);
}

StreamtasksClient StreamtasksClient::connect_unix(const char* socket_path)
{
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        throw new std::runtime_error("Failed to open socket file");
    }
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        close(sockfd);
        throw new std::runtime_error("Failed to connect to socket.");
    }

    return StreamtasksClient(sockfd);
}
