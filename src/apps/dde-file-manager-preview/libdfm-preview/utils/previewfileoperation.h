// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWFILEOPERATION_H
#define PREVIEWFILEOPERATION_H

#include "dfmplugin_filepreview_global.h"

#include <QObject>

namespace dfmplugin_filepreview {
class PreviewFileOperation : public QObject
{
    Q_OBJECT
public:
    explicit PreviewFileOperation(QObject *parent = nullptr);

public:
    static bool openFileHandle(quint64 winID, const QUrl &url);
};
}
#endif   // PREVIEWFILEOPERATION_H
