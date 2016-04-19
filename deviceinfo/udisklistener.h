#ifndef UDISKLISTENER_H
#define UDISKLISTENER_H

#include "../../filemanager/controllers/abstractfilecontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusObjectPath>
#include <QList>
#include <QMap>
#include <QDBusArgument>
#include <QXmlStreamReader>
#include <QDBusPendingReply>

class UDiskDeviceInfo;

class UDiskListener : public AbstractFileController
{
    Q_OBJECT
public:
    UDiskListener();
    UDiskDeviceInfo * getDevice(const QDBusObjectPath &path) const;
    void addDevice(UDiskDeviceInfo * device);
    void removeDevice(UDiskDeviceInfo * device);
    void update();
    QString lastPart(const QString &path);
    bool isSystemDisk(const QString &path) const;
public slots:
    void interfacesAdded(const QDBusObjectPath &path, const QMap<QString, QVariant> &interfaces);
    void interfacesRemoved(const QDBusObjectPath &path, const QStringList &interfaces);
    void interfacesChanged();
    void mount(const QString &path);
    void unmount(const QString &path);
private:
    void readFstab();
    QList<UDiskDeviceInfo *> m_list;
    QMap<QString, UDiskDeviceInfo *> m_map;
    QList<QString> fstab;
public:
    const QList<AbstractFileInfo *> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const;
    AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const;
};

#endif // UDISKLISTENER_H
