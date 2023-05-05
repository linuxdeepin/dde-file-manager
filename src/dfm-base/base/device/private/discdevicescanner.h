// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCDEVICESCANNER_H
#define DISCDEVICESCANNER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QTimer>
#include <QDBusVariant>
#include <QThreadPool>

namespace dfmbase {

namespace DiscDevice {
class Scanner : public QRunnable
{
public:
    explicit Scanner(const QString &dev);
    void run() override;

private:
    QString device;
};
}   // namespace namespace

class DiscDeviceScanner final : public QObject
{
    Q_OBJECT

public:
    explicit DiscDeviceScanner(QObject *parent = nullptr);

    void initialize();

private:
    bool startScan();
    void stopScan();
    void updateScanState();

private Q_SLOTS:
    void scanOpticalDisc();
    void onDevicePropertyChangedQVar(const QString &id, const QString &propertyName, const QVariant &var);
    void onDevicePropertyChangedQDBusVar(const QString &id, const QString &propertyName, const QDBusVariant &var);
    void onDiscWorkingStateChanged(const QString &id, const QString &dev, bool working);

private:
    QScopedPointer<QTimer> discScanTimer { new QTimer };
    QScopedPointer<QThreadPool> threadPool { new QThreadPool };
    QStringList discDevIdGroup;
};

}   // namespace dfmbase

#endif   // DISCDEVICESCANNER_H
