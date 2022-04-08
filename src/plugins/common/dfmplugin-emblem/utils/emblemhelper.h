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
#ifndef EMBLEMHELPER_H
#define EMBLEMHELPER_H

#include "dfmplugin_emblem_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include <dfm-framework/framework.h>

#include <QIcon>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemHelper : public QObject
{
public:
    explicit EmblemHelper(QObject *parent);

    QList<QIcon> getSystemEmblems(const AbstractFileInfoPointer &info) const;
    QList<QIcon> getCustomEmblems(const AbstractFileInfoPointer &info) const;
    QMap<int, QIcon> getGioEmblems(const AbstractFileInfoPointer &info) const;
    QList<QIcon> getExtendEmblems(const AbstractFileInfoPointer &info) const;

    static DPF_NAMESPACE::EventSequenceManager *eventSequence();

private:
    QIcon standardEmblem(const SystemEmblemType type) const;

    bool parseEmblemString(QIcon &emblem, QString &pos, const QString &emblemStr) const;
    void setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> &iconMap) const;
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEMHELPER_H
