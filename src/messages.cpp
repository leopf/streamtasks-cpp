#include "streamtasks.hpp"

StreamtasksMessage::StreamtasksMessage(MsgPack content)
{
    this->content = content;
}

std::vector<uint8_t> StreamtasksMessage::data()
{
    return content["data"].binary_items();
}

unsigned long StreamtasksMessage::topic()
{
    return content["topic"].int_value();
}

bool StreamtasksMessage::paused()
{
    return content["paused"].bool_value();
}

unsigned long StreamtasksMessage::address()
{
    return content["address"].int_value();
}
unsigned long StreamtasksMessage::port()
{
    return content["port"].int_value();
}

void StreamtasksMessage::print()
{
    switch (content["_id"].int_value())
    {
    case 0:
        printf("TopicDataMessage");
        break;
    case 1:
        printf("TopicControlMessage");
        break;
    case 2:
        printf("AddressedMessage");
        break;
    case 3:
        printf("AddressesChangedMessage");
        break;
    case 4:
        printf("InTopicsChangedMessage");
        break;
    case 5:
        printf("OutTopicsChangedMessage");
        break;
    default:
        throw std::runtime_error("unknown message type!");
    }
    printf("\n");
}