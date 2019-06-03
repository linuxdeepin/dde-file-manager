/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
                    if (obj.contains("rm")) {
                        // blumia: `lsblk -J` may return value like `1`(old behavior) or `true`(changed in util-linux v2.33).
                        //         So we convert it to QVariant and use its toBool() to make sure the result is correct.
                        QVariant data(obj.value("rm").toVariant());
                        p.setIsRemovable(data.toBool());
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
                        qDebug() << "read usage of" << p.path() << ret;
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

Partition Partition::getPartitionByMountPoint(const QString &mountPoint)
{
    Partition ret;
    QString output;
    QString err;

    bool status = SpawnCmd("lsblk", {"-pOJ"},
                  output, err);
    auto parseObject = [&ret](const QJsonObject &obj){
        if (obj.contains("name")){
            ret.setName(obj.value("name").toString());
            ret.setPath(obj.value("name").toString());
        }
        if (obj.contains("fstype")){
            ret.setFs(obj.value("fstype").toString());
        }
        if (obj.contains("label")){
            ret.setLabel(obj.value("label").toString());
        }
        if (obj.contains("uuid")){
            ret.setUuid(obj.value("uuid").toString());
        }
        if (obj.contains("mountpoint")){
            ret.setMountPoint(obj.value("mountpoint").toString());
        }
        if(obj.contains("rm")){
            QVariant data(obj.value("rm").toVariant());
            ret.setIsRemovable(data.toBool());
        }

        if (!ret.fs().isEmpty()){
            ReadUsageManager readUsageManager;
            qlonglong freespace = 0;
            qlonglong total = 0;
            bool r = readUsageManager.readUsage(ret.path(), ret.fs(), freespace, total);
            if (r){
                ret.setFreespace(freespace);
                ret.setTotal(total);
            }
            qDebug() << "read usage of" << ret.path() << r;
        }
    };
    if(status){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(output.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject devObj = doc.object();
            foreach (QString key, devObj.keys()) {
                if (key == "blockdevices"){
                    QJsonArray arr = devObj.value(key).toArray();
                    for (auto i = arr.begin(); i != arr.end(); ++i) {
                        QJsonObject io = i->toObject();
                        if (io.contains("mountpoint") && io["mountpoint"].toString() == mountPoint) {
                            parseObject(io);
                            break;
                        }

                        if (io.contains("children") && io["children"].isArray()) {
                            QJsonArray children = io["children"].toArray();
                            bool f = false;
                            for (auto j = children.begin(); j != children.end(); ++j) {
                                QJsonObject jo = j->toObject();
                                if (jo.contains("mountpoint") && jo["mountpoint"].toString() == mountPoint) {
                                    parseObject(jo);
                                    f = true;
                                    break;
                                }
                            }
                            if (f) {
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }else{
            qDebug() << error.errorString();
        }
    }else{
        qDebug() << status << output << err;
    }
    return ret;
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
