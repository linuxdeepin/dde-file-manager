// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNEVENTRECEIVER_H
#define BURNEVENTRECEIVER_H

#include "dfmplugin_burn_global.h"

#include <QObject>

namespace dfmplugin_burn {

class BurnEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BurnEventReceiver)

public:
    static BurnEventReceiver *instance();

public slots:
    void handleShowBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent);
    void handleShowDumpISODlg(const QString &devId);
    void handleErase(const QString &dev);
    void handlePasteTo(const QList<QUrl> &urls, const QUrl &dest, bool isCopy);
    void handleCopyFilesResult(const QList<QUrl> &srcUrls,
                               const QList<QUrl> &destUrls,
                               bool ok,
                               const QString &errMsg);
    void handleMountImage(quint64 winId, const QUrl &isoUrl);

private:
    explicit BurnEventReceiver(QObject *parent = nullptr);
};

}

#endif   // BURNEVENTRECEIVER_H
