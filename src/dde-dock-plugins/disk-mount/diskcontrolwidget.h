// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <dfmglobal.h>
#include <dgiomount.h>

class DDiskManager;
DFM_BEGIN_NAMESPACE
class DFMSettings;
class DFMVfsManager;
DFM_END_NAMESPACE

class DGioVolumeManager;
class DUMountManager;
class DiskControlItem;

class DiskControlWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit DiskControlWidget(QWidget *parent = nullptr);
    ~DiskControlWidget() override;
    void initConnect();

    DDiskManager *startMonitor();
    void doStartupAutoMount();
    void unmountAll();
    void doUnMountAll();

    const QList<QExplicitlySharedDataPointer<DGioMount>> getVfsMountList();
    static void NotifyMsg(QString msg);
    static void NotifyMsg(QString title, QString msg);
    static void refreshDesktop();

signals:
    void diskCountChanged(const int count) const;

private:
    void popQueryScanningDialog(QObject *object, std::function<void()> onStop);

private slots:
    void onDriveConnected(const QString &deviceId);
    void onDiskListChanged();
    void onDriveDisconnected();
    void onMountAdded();
    void onMountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint);
    void onVolumeAdded();
    void onVolumeRemoved();
    void onVfsMountChanged(QExplicitlySharedDataPointer<DGioMount> mount);
    void onBlockDeviceAdded(const QString &path);
    void onItemUmountClicked(DiskControlItem *item);

private:
    QVBoxLayout *m_centralLayout;
    QWidget *m_centralWidget;
    bool m_isInLiveSystem = false;   // 当处于 liveSys 的时候禁用自动挂载（以前的逻辑）
    bool m_autoMountEnable = false;   // 配置项中的自动挂载
    bool m_autoMountAndOpenEnable = false;   // 配置项中的挂载并打开

    DDiskManager *m_diskManager;
    QScopedPointer<DUMountManager> m_umountManager;
    QScopedPointer<DGioVolumeManager> m_vfsManager;
};

#endif   // DISKCONTROLWIDGET_H
