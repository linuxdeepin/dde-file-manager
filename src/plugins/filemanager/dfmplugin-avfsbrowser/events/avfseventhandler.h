// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSEVENTHANDLER_H
#define AVFSEVENTHANDLER_H

#include "dfmplugin_avfsbrowser_global.h"

#include <QUrl>
#include <QList>
#include <QObject>

namespace dfmplugin_avfsbrowser {

class AvfsEventHandler : public QObject
{
    AvfsEventHandler() { }
    Q_DISABLE_COPY(AvfsEventHandler)

public:
    static AvfsEventHandler *instance();

    bool hookOpenFiles(quint64 winId, const QList<QUrl> &urls);
    bool hookEnterPressed(quint64 winId, const QList<QUrl> &urls);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);

    void openArchivesAsDir(quint64 winId, const QList<QUrl> &urls);
    void writeToClipbord(quint64 winId, const QList<QUrl> &urls);
    void showProperty(const QList<QUrl> &urls);
};

}

#endif   // AVFSEVENTHANDLER_H
