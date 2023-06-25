// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QPointer>
#include <DDialog>

#include <mutex>

using namespace DTK_NAMESPACE::Widget;
class DiskControlItem;

class DiskControlWidget : public QScrollArea
{
    friend class DiskControlItem;
    Q_OBJECT
public:
    explicit DiskControlWidget(QWidget *parent = nullptr);
    void initListByMonitorState();

signals:
    void diskCountChanged(const int count) const;

private slots:
    void onDiskListChanged();
    void onAskStopScanning(const QString &method, const QString &id);
    void onDeviceBusy(int action);

private:
    void initializeUi();
    void initConnection();
    void removeWidgets();
    void paintUi();
    void addSeparateLineUi(int width);
    int addBlockDevicesItems();
    int addProtocolDevicesItems();
    int addItems(const QStringList &list, bool isBlockDevice);
    DDialog *showQueryScanningDialog(const QString &title);
    void handleWhetherScanning(const QString &method, const QString &id);
    void notifyMessage(const QString &msg);
    void notifyMessage(const QString &title, const QString &msg);

    static std::once_flag &initOnceFlag();
    static std::once_flag &retryOnceFlag();

private:
    QVBoxLayout *centralLayout { nullptr };
    QWidget *centralWidget { nullptr };
};

#endif   // DISKCONTROLWIDGET_H
