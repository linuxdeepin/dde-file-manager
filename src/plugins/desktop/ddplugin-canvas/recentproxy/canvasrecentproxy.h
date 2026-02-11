// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASRECENTPROXY_H
#define CANVASRECENTPROXY_H

#include <QObject>

namespace ddplugin_canvas {

class CanvasRecentProxy : public QObject
{
    Q_OBJECT
public:
    explicit CanvasRecentProxy(QObject *parent = nullptr);
    ~CanvasRecentProxy() override;

public slots:
    void handleReloadRecentFiles(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg);
};

} // namespace ddplugin_canvas

#endif // CANVASRECENTPROXY_H
