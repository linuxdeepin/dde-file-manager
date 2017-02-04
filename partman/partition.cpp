#include "partition.h"
#include "command.h"
#include "readusagemanager.h"
#include <QString>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

namespace PartMan {


Partition::Partition()
{

}

Partition Partition::getPartitionByDevicePath(const QString &devicePath)
{
    Partition p;
    p.setPath(devicePath);
    QString output;
    QString err;

    bool status = SpawnCmd("lsblk", {"-O", "-J", devicePath},
                  output, err);
    if(status){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(output.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject devObj = doc.object();
            foreach (QString key, devObj.keys()) {
                if (key == "blockdevices"){
                    QJsonObject obj = devObj.value(key).toArray().at(0).toObject();
                    if (obj.contains("name")){
                        p.setName(obj.value("name").toString());
                    }
                    if (obj.contains("fstype")){
                        p.setFs(obj.value("fstype").toString());
                    }
                    if (obj.contains("label")){
                        p.setLabel(obj.value("label").toString());
                    }
                    if (obj.contains("uuid")){
                        p.setUuid(obj.value("uuid").toString());
                    }
                    if (obj.contains("mountpoint")){
                        p.setMountPoint(obj.value("mountpoint").toString());
                    }
                    if(obj.contains("rm")){
                        QString data = obj.value("rm").toString();
                        if(data == "1")
                            p.setIsRemovable(true);
                        else
                            p.setIsRemovable(false);
                    }

                    if (!p.fs().isEmpty()){
                        ReadUsageManager readUsageManager;
                        qlonglong freespace = 0;
                        qlonglong total = 0;
                        bool ret = readUsageManager.readUsage(p.path(), p.fs(), freespace, total);
                        if (ret){
                            p.setFreespace(freespace);
                            p.setTotal(total);
                        }
                        qDebug() << "read usgae of" << p.path() << ret;
                    }
                }
            }
        }else{
            qDebug() << error.errorString();
        }
    }else{
        qDebug() << status << output << err;
    }
    return p;
}

QString Partition::path() const
{
    return m_path;
}

void Partition::setPath(const QString &path)
{
    m_path = path;
}

QString Partition::fs() const
{
    return m_fs;
}

void Partition::setFs(const QString &fs)
{
    m_fs = fs;
}

QString Partition::label() const
{
    return m_label;
}

void Partition::setLabel(const QString &label)
{
    m_label = label;
}

QString Partition::name() const
{
    return m_name;
}

void Partition::setName(const QString &name)
{
    m_name = name;
}

QString Partition::uuid() const
{
    return m_uuid;
}

void Partition::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString Partition::mountPoint() const
{
    return m_mountPoint;
}

void Partition::setMountPoint(const QString &mountPoint)
{
    m_mountPoint = mountPoint;
}

bool Partition::getIsRemovable() const
{
    return m_isRemovable;
}

void Partition::setIsRemovable(bool isRemovable)
{
    m_isRemovable = isRemovable;
}

qlonglong Partition::freespace() const
{
    return m_freespace;
}

void Partition::setFreespace(const qlonglong &freespace)
{
    m_freespace = freespace;
}

qlonglong Partition::total() const
{
    return m_total;
}

void Partition::setTotal(const qlonglong &total)
{
    m_total = total;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Partition &obj)
{
    argument.beginStructure();
    argument << obj.m_path;
    argument << obj.m_fs;
    argument << obj.m_label;
    argument << obj.m_name;
    argument << obj.m_uuid;
    argument << obj.m_mountPoint;
    argument << obj.m_isRemovable;
    argument << obj.m_freespace;
    argument << obj.m_total;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Partition &obj)
{
    argument.beginStructure();
    argument >> obj.m_path;
    argument >> obj.m_fs;
    argument >> obj.m_label;
    argument >> obj.m_name;
    argument >> obj.m_uuid;
    argument >> obj.m_mountPoint;
    argument >> obj.m_isRemovable;
    argument >> obj.m_freespace;
    argument >> obj.m_total;
    argument.endStructure();
    return argument;
}

void Partition::registerMetaType()
{
    qRegisterMetaType<Partition>(QT_STRINGIFY(Partition));
    qDBusRegisterMetaType<Partition>();
}

QDebug operator<<(QDebug dbg, const Partition &partion)
{
    dbg << "Partition: {"
        << "path:" << partion.path()
        << "name:" << partion.name()
          << "fstype:" << partion.fs()
          << "label:" << partion.label()
          << "uuid:" << partion.uuid()
          << "mountPoint:" << partion.mountPoint()
          << "freespace:" << partion.freespace()
          << "total:" << partion.total()
          << "}";
     return dbg;
}

}
