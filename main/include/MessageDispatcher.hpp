#pragma once

#include <vector>
#include <set>

#include "ITask.hpp"
#include "MessageConsumer.hpp"
#include "singletonTemplate.hpp"

using namespace std;

// typedef xQueueHandle subscriberQueue;

typedef struct {
    eMessageCode topic;
    set<MessageConsumer*> subscribers;
}topicSubscribersList_t;

class MessageDispatcher : public Singleton<MessageDispatcher>, public ITask, public MessageConsumer{
    friend class Singleton;
    public:
        void addSubscriber(const eMessageCode topic, MessageConsumer* const sub);
    private:
        MessageDispatcher();
        topicSubscribersList_t& findOrCreateTopicSubscribersList(const eMessageCode topic);
        void handleIncommingMessage (const message_t& message);
        void run(void* data) override;
    private:
        vector<topicSubscribersList_t> _topicSubList;
};
#define DISPATCHER MessageDispatcher::instance()