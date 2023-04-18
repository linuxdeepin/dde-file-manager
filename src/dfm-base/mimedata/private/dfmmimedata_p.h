// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMIMEDATA_P_H
#define DFMMIMEDATA_P_H

#include <dfm-base/dfm_base_global.h>

#include <QSharedData>
#include <QVariantMap>

namespace dfmbase {

class DFMMimeDataPrivate : public QSharedData
{
public:
    explicit DFMMimeDataPrivate();
    DFMMimeDataPrivate(const DFMMimeDataPrivate &other);
    ~DFMMimeDataPrivate();

    void parseUrls(const QList<QUrl> &urls);

public:
    QVariantMap attributes;
    QString version;

    QList<QUrl> urlList;
};

}   // namespace dfmbase

#endif   // DFMMIMEDATA_P_H
