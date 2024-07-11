#include "streamtasks.hpp"

MsgPack::array ids_to_priced_id_array(std::vector<uint64_t>* ids) 
{
    MsgPack::array result;
    for (size_t i = 0; i < ids->size(); i++)
    {
        result.push_back(MsgPack::object { { "id", ids->at(i) }, { "cost", 0 } });
    }
    return result;
}

MsgPack::binary string_to_msg_pack_bin(std::string s) 
{
    MsgPack::binary res(s.begin(), s.end());
    return res;
}

StreamtasksMessage::StreamtasksMessage(MsgPack content)
{
    this->content = content;
}

MsgPack StreamtasksMessage::data()
{
    std::string err;
    std::vector<uint8_t> bin = content["data"].binary_items();
    return MsgPack::parse((char*)bin.data(), bin.size(), err);
}

uint64_t StreamtasksMessage::topic()
{
    return content["topic"].uint64_value();
}

bool StreamtasksMessage::paused()
{
    return content["paused"].bool_value();
}

uint64_t StreamtasksMessage::address()
{
    return content["address"].uint64_value();
}
uint64_t StreamtasksMessage::port()
{
    return content["port"].uint64_value();
}

std::string StreamtasksMessage::serialize()
{
    return content.dump();
}

StreamtasksMessageID StreamtasksMessage::id()
{
    return static_cast<StreamtasksMessageID>(content["_id"].int_value());
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

StreamtasksMessage StreamtasksMessage::topic_data_message(uint64_t topic, MsgPack data)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_TOPIC_DATA },
        { "topic", topic },
        { "data", string_to_msg_pack_bin(data.dump()) },
    });
}

StreamtasksMessage StreamtasksMessage::topic_control_message(uint64_t topic, TopicControlData control)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_TOPIC_CONTROL },
        { "topic", topic },
        { "paused", control.paused },
    });
}

StreamtasksMessage StreamtasksMessage::addressed_message(uint64_t address, uint64_t port, MsgPack data)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_ADDRESSED },
        { "address", address },
        { "port", port },
        { "data", string_to_msg_pack_bin(data.dump()) },
    });
}

StreamtasksMessage StreamtasksMessage::addresses_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_ADDRESSES_CHANGED },
        { "add", MsgPack::array(ids_to_priced_id_array(&add)) },
        { "remove", remove },
    });
}

StreamtasksMessage StreamtasksMessage::in_topics_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_IN_TOPICS_CHANGED },
        { "add", add },
        { "remove", remove },
    });
}
StreamtasksMessage StreamtasksMessage::out_topics_changed_message(std::vector<uint64_t> add, std::vector<uint64_t> remove)
{
    return StreamtasksMessage(MsgPack::object {
        { "_id", StreamtasksMessageID::MSG_ID_OUT_TOPICS_CHANGED },
        { "add", ids_to_priced_id_array(&add) },
        { "remove", remove },
    });
}
