#include "partition.h"
#include "command.h"
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

    bool status = SpawnCmd("lsblk", {"-f", "-J", devicePath},
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

QDebug operator<<(QDebug dbg, const Partition &partion)
{
    dbg << "Partition: {"
          << "path:" << partion.path()
          << "name:" << partion.name()
          << "fstype:" << partion.fs()
          << "label:" << partion.label()
          << "uuid:" << partion.uuid()
          << "mountPoint:" << partion.mountPoint()
          << "}";
     return dbg;
}

}
