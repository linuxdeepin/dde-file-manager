#ifndef FILEEVENTRECEIVER_H
#define FILEEVENTRECEIVER_H

#include "dfm_common_service_global.h"

#include <dfm-framework/event/eventcallproxy.h>
#include <QString>

DSC_BEGIN_NAMESPACE

namespace FileEventTypes
{
const QString TOPIC_FILE_EVENT {"fileEvent"};
const QString DATA_MKDIR {"mkdir"};
const QString DATA_TOUCH {"touch"};
const QString PROPERTY_KEY_ROOTURL {"rootUrl"};
const QString PROPERTY_KEY_DIRNAMES {"dirNames"};
const QString PROPERTY_KEY_DIRNAME {"dirName"};
} //FileEventTypes

class FileEventReceiver final : public dpf::EventHandler, dpf::AutoEventHandlerRegister<FileEventReceiver>
{
    Q_OBJECT
public:
    static EventHandler::Type type();
    static QStringList topics();
    explicit FileEventReceiver() : AutoEventHandlerRegister<FileEventReceiver>() {}
    virtual void eventProcess(const dpf::Event &event) override;
};

DSC_END_NAMESPACE

#endif // FILEEVENTRECEIVER_H
