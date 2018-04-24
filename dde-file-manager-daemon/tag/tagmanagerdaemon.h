#ifndef TAGMANAGERDAEMON_H
#define TAGMANAGERDAEMON_H

#include <memory>

#include <QMap>
#include <QList>
#include <QObject>


#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

class TagManagerDaemonAdaptor;


class TagManagerDaemon : public QObject
{
    Q_OBJECT
public:
    TagManagerDaemon(QObject* const parent);
    virtual ~TagManagerDaemon()=default;
    TagManagerDaemon(const TagManagerDaemon& other)=delete;
    TagManagerDaemon& operator=(const TagManagerDaemon& other)=delete;

public slots:
    QDBusVariant disposeClientData(const QMap<QString, QVariant>& filesAndTags, const unsigned long long& type);

private:
    TagManagerDaemonAdaptor* adaptor{ nullptr };
};
#endif // TAGMANAGERDAEMON_H
