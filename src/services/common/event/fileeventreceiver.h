#ifndef FILEEVENTRECEIVER_H
#define FILEEVENTRECEIVER_H

#include "dfm_common_service_global.h"
#include "dfm-framework/event/eventcallproxy.h"
#include <QString>

DSC_BEGIN_NAMESPACE

namespace FileEventTypes
{
const QString TOPIC_FILE_EVENT {"fileEvent"};
const QString DATA_MKDIR {"mkdir"};
const QString PROPERTY_KEY_URLS {"urls"};
const QString PROPERTY_KEY_URL {"url"};
} //FileEventTypes

class FileEventReceiver final : public dpf::AsyncEventHandler
{
    Q_OBJECT
public:
    explicit FileEventReceiver();
    virtual void eventProcess(const dpf::Event &event) override;
};

DSC_END_NAMESPACE

#endif // FILEEVENTRECEIVER_H
