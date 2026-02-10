// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAINFOFETCHWORKER_H
#define MEDIAINFOFETCHWORKER_H
#include "dfmplugin_propertydialog_global.h"

#include <QObject>

namespace dfmplugin_propertydialog {
class MediaInfoFetchWorker : public QObject
{
    Q_OBJECT
public:
    explicit MediaInfoFetchWorker(QObject *parent = nullptr);

public Q_SLOTS:
    void getDuration(const QString &filePath);

Q_SIGNALS:
    void durationReady(const QString &duration);

private:
    bool hasFFmpeg();
};
} // namespace dfmplugin_propertydialog

#endif   // MEDIAINFOFETCHWORKER_H
