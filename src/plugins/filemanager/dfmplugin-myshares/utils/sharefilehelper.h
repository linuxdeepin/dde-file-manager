// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREFILEHELPER_H
#define SHAREFILEHELPER_H

#include "dfmplugin_myshares_global.h"

#include <QObject>

DPMYSHARES_BEGIN_NAMESPACE

class ShareFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ShareFileHelper)
public:
    static ShareFileHelper *instance();

    // file operation
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> &urls);

private:
    explicit ShareFileHelper(QObject *parent = nullptr);
};
DPMYSHARES_END_NAMESPACE

#endif   // SHAREFILEHELPER_H
