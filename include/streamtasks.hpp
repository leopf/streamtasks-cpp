class StreamtasksClient
{
private:
    int sockfd;
public:
    StreamtasksClient(int sockfd);
    ~StreamtasksClient();

    static StreamtasksClient connect_unix(const char* socket_path);
};


