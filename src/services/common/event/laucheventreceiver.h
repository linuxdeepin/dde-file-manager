#ifndef LAUCHEVENTRECEIVER_H
#define LAUCHEVENTRECEIVER_H

#include "dfm-framework/event/eventhandler.h"

namespace LauchEventTypes
{
const QString TOPIC_FILE_EVENT {"lauchEvent"};
const QString DATA_OPEN_BY_APP {"openByApp"};
const QString PROPERTY_KEY_URLS {"urls"};
const QString PROPERTY_KEY_URL {"url"};
} //FileEventTypes

class LauchEventReceiver final : public dpf::AsyncEventHandler
{
    Q_OBJECT
public:
    explicit LauchEventReceiver();
    virtual void eventProcess(const dpf::Event &event) override;
    void openByApp(const dpf::Event &event);
};
#endif // LAUCHEVENTRECEIVER_H
