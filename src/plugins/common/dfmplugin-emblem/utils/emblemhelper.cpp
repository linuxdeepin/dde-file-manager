/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "emblemhelper.h"

#include "dfm-base/utils/decorator/decoratorfileinfo.h"

#include <dfm-framework/event/event.h>
#include <dfm-io/core/dfileinfo.h>

#include <QDebug>
#include <QStandardPaths>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

EmblemHelper::EmblemHelper(QObject *parent)
    : QObject(parent)
{
}

QList<QIcon> EmblemHelper::getSystemEmblems(const AbstractFileInfoPointer &info) const
{
    QList<QIcon> emblems;

    if (info->isSymLink())
        emblems << QIcon::fromTheme("emblem-symbolic-link", standardEmblem(SystemEmblemType::kLink));

    if (!info->isWritable())
        emblems << QIcon::fromTheme("emblem-readonly", standardEmblem(SystemEmblemType::kLock));

    if (!info->isReadable())
        emblems << QIcon::fromTheme("emblem-unreadable", standardEmblem(SystemEmblemType::kUnreadable));

    bool shared = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_IsPathShared", info->absoluteFilePath()).toBool();
    if (shared)
        emblems << QIcon::fromTheme("emblem-shared", standardEmblem(SystemEmblemType::kShare));

    return emblems;
}

QMap<int, QIcon> EmblemHelper::getGioEmblems(const AbstractFileInfoPointer &info) const
{
    QMap<int, QIcon> emblemsMap;

    DecoratorFileInfo fileInfo(info->url());
    QStringList emblemData = fileInfo.customAttribute("metadata::emblems", DFileInfo::DFileAttributeType::kTypeStringV).toStringList();

    if (emblemData.isEmpty())
        return emblemsMap;

    QString emblemsStr = emblemData.first();

    if (!emblemsStr.isEmpty()) {
        QStringList emblemsStrList = emblemsStr.split("|", QString::SkipEmptyParts);
        for (int i = 0; i < emblemsStrList.length(); i++) {
            QString pos;
            QIcon emblem;
            if (parseEmblemString(emblem, pos, emblemsStrList.at(i)))
                setEmblemIntoIcons(pos, emblem, emblemsMap);
        }

        return emblemsMap;
    }

    return emblemsMap;
}

QIcon EmblemHelper::standardEmblem(const SystemEmblemType type) const
{
    static QIcon linkEmblem(QIcon::fromTheme("emblem-symbolic-link"));
    static QIcon lockEmblem(QIcon::fromTheme("emblem-locked"));
    static QIcon unreadableEmblem(QIcon::fromTheme("emblem-unreadable"));
    static QIcon shareEmblem(QIcon::fromTheme("emblem-shared"));

    switch (type) {
    case SystemEmblemType::kLink:
        return linkEmblem;
    case SystemEmblemType::kLock:
        return lockEmblem;
    case SystemEmblemType::kUnreadable:
        return unreadableEmblem;
    case SystemEmblemType::kShare:
        return shareEmblem;
    }

    return QIcon();
}

bool EmblemHelper::parseEmblemString(QIcon &emblem, QString &pos, const QString &emblemStr) const
{
    // default position
    pos = "rd";

    if (!emblemStr.isEmpty()) {
        QIcon emblemIcon;
        QString imgPath;

        if (emblemStr.contains(";")) {
            QStringList emStrList = emblemStr.split(";");
            imgPath = emStrList.at(0);
            pos = emStrList.at(1);
        } else {
            imgPath = emblemStr;
        }

        // to standard path
        if (imgPath.startsWith("~/"))
            imgPath.replace(0, 1, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

        DecoratorFileInfo fileInfo(imgPath);

        if (fileInfo.exists()) {
            if (fileInfo.size() > 102400)   // size small than 100kb
                return false;

            QString suffix = fileInfo.completeSuffix();
            // check support type
            if (suffix != "svg" && suffix != "png" && suffix != "gif" && suffix != "bmp" && suffix != "jpg")
                return false;

            emblemIcon = QIcon(imgPath);
            if (!emblemIcon.isNull()) {
                emblem = emblemIcon;
                return true;
            }
        }
    }

    return false;
}

void EmblemHelper::setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> &iconMap) const
{
    int emblemIndex = 0;   // default position rd = 0, rightdown

    // left down
    if (pos == "ld")
        emblemIndex = 1;
    // left up
    else if (pos == "lu")
        emblemIndex = 2;
    // right up
    else if (pos == "ru")
        emblemIndex = 3;

    iconMap[emblemIndex] = emblem;
}
