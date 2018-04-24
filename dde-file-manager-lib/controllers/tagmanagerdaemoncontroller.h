#ifndef TAGMANAGERDAEMONCONTROLLER_H
#define TAGMANAGERDAEMONCONTROLLER_H

#include <memory>


#include "tag/tagutil.h"
#include "./interface/tagmanagerdaemon_interface.h"

#include <QObject>
#include <QVariantMap>

class TagManagerDaemonController final : public QObject
{
    Q_OBJECT
public:
    virtual ~TagManagerDaemonController()=default;
    TagManagerDaemonController(QObject* const parent = nullptr);
    TagManagerDaemonController(const TagManagerDaemonController& other)=delete;
    TagManagerDaemonController& operator=(const TagManagerDaemonController& other)=delete;

    QVariant disposeClientData(const QVariantMap& filesAndTags, Tag::ActionType type);

    static TagManagerDaemonController* instance();
private:
    std::unique_ptr<TagManagerDaemonInterface> m_daemonInterface{ nullptr };
};


#endif // TAGMANAGERDAEMONCONTROLLER_H
