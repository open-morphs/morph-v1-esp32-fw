#include "MessageDispatcher.hpp"
#include "Utils/threadSafeDbg.hpp"

static const uint32_t dispatcherQueueLength {15};
static const char* const TAG = "DISPATCHER";
static const uint32_t defaultSendTimeoutMs{30};

MessageDispatcher::MessageDispatcher() 
                :   MessageConsumer(dispatcherQueueLength) {
    setName("Message Dispatcher");
    setPriority(10);
    setStackSize(configMINIMAL_STACK_SIZE*3);
}

void MessageDispatcher::addSubscriber(const eMessageCode topic, MessageConsumer* const sub) {

    topicSubscribersList_t& sublist = findOrCreateTopicSubscribersList(topic);
    sublist.subscribers.insert(sub);
    DBG_PRINT_TAG(TAG, "new subscribtion added for topic: %u", topic);
}

topicSubscribersList_t& MessageDispatcher::findOrCreateTopicSubscribersList(const eMessageCode topic) {

    for (auto &sublist : _topicSubList) {
        if (sublist.topic == topic) {
            return sublist;
        }
    }
    
    topicSubscribersList_t newSublist = {
        .topic = topic,
        .subscribers = {}
    };

    _topicSubList.push_back(newSublist);

    return _topicSubList.back();
}

void MessageDispatcher::run(void* data) {

    static message_t message {};
    while (1) {
        consumeMessage(message);
        handleIncommingMessage(message);
    }
}

void MessageDispatcher::handleIncommingMessage (const message_t& message) {
    topicSubscribersList_t subList = findOrCreateTopicSubscribersList(message.messageCode);
    for(auto &sub : subList.subscribers){
        BaseType_t sendResult {xQueueSendToBack(sub->getRxQueue(), &message, defaultSendTimeoutMs/portTICK_RATE_MS)};
        /*  Should never fail! Otherwise will lead to race condition,
            when subs try to write to the full RX dispatcher's queue,
            and dispatcher can't handle subs, with the same reason
        */
        if(sendResult != pdTRUE) {
            printf("CRITICAL ERROR: failed to handle message: %u \r\n", message.messageCode);
            assert(false);
        }
    }
}