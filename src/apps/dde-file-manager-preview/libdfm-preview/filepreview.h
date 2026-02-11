// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEW_H
#define FILEPREVIEW_H

#include "dfmplugin_filepreview_global.h"
#include <QObject>

namespace dfmplugin_filepreview {
class FilePreview : public QObject
{
    Q_OBJECT
public:
    void initialize();
    bool start();
    void showFilePreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);

private:
    bool isPreviewEnabled();
};
}   // namespace dfmplugin_filepreview
#endif   // FILEPREVIEW_H
