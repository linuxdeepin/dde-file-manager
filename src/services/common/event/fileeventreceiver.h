#ifndef FILEEVENTRECEIVER_H
#define FILEEVENTRECEIVER_H

#include "dfm_common_service_global.h"

#include <dfm-framework/event/eventcallproxy.h>
#include <QString>

DSC_BEGIN_NAMESPACE

extern const QString TOPIC_FILE_EVENT;

namespace FileEventDatas {
extern const QString MKDIR;
extern const QString TOUCH;
} // namespace FileEventDatas

namespace FileEvenPropertys {
extern const QString ROOTURL;
extern const QString DIRNAMES;
extern const QString DIRNAME;
} // namespace FileEvenPropertys

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
