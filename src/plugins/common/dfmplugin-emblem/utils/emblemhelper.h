// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMHELPER_H
#define EMBLEMHELPER_H

#include "dfmplugin_emblem_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-framework/dpf.h>

#include <QIcon>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemHelper : public QObject
{
public:
    explicit EmblemHelper(QObject *parent);

    QList<QIcon> getSystemEmblems(const AbstractFileInfoPointer &info) const;
    QMap<int, QIcon> getGioEmblems(const AbstractFileInfoPointer &info) const;

private:
    QIcon standardEmblem(const SystemEmblemType type) const;

    bool parseEmblemString(QIcon &emblem, QString &pos, const QString &emblemStr) const;
    void setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> &iconMap) const;
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEMHELPER_H
