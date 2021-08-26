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

#ifndef FMEVENT_H
#define FMEVENT_H

#include "dfm-base/base/dfmglobal.h"

#include <QUrl>
#include <QString>
#include <QSharedData>
#include <QMetaType>
#include <QEvent>
#include <QPointer>
#include <QDir>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QModelIndex>
#include <functional>

class DFMEvent
{
    Q_GADGET
    QDOC_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)

public:
    enum Type {
        UnknowType = 0,          //未知事件类型
        FileEvent = 1,           //文件操作事件（创建文件，删除文件等）
        LauchEvent = 2000,       //文件与App交互事件（使用其他程序打开本地文件）
        WindowEvent = 3000,      //窗口事件（文件窗口点击等触发事件，导航操作等）
        CustomEvent = 4000,      //更多自定义扩展项
    };
    Q_ENUM(Type)

    explicit DFMEvent(int type);

    DFMEvent(const DFMEvent &other);

    virtual ~DFMEvent();

    DFMEvent &operator =(const DFMEvent &other);

    void setAccepted(bool accepted);

    bool isAccepted() const;

    int type() const;

    void setType(int type);

protected:

    int m_type;
    bool m_accept;
};

typedef QSharedPointer<DFMEvent> DFMEventPointer;

#endif // FMEVENT_H
