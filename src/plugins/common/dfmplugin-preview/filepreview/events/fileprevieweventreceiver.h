// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWEVENTRECEIVER_H
#define FILEPREVIEWEVENTRECEIVER_H
#include "dfmplugin_filepreview_global.h"

#include <QObject>

namespace dfmplugin_filepreview {
class FilePreviewEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FilePreviewEventReceiver)
private:
    explicit FilePreviewEventReceiver(QObject *parent = nullptr);

public:
    static FilePreviewEventReceiver *instance();
    void connectService();

public slots:
    void showFilePreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);
};
}
#endif
