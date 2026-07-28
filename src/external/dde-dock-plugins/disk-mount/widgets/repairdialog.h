// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPAIRDIALOG_H
#define REPAIRDIALOG_H

#include <DDialog>
#include <DWaterProgress>

#include <QMap>
#include <QTimer>

class RepairDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    enum RepairState {
        kConfirm,       // 确认是否修复
        kRepairing,     // 修复进行中
        kSuccess,       // 修复成功
        kFailed         // 修复失败
    };

    explicit RepairDialog(QWidget *parent = nullptr);

    void setDeviceInfo(const QString &deviceName, const QString &deviceSize, const QString &fsType);
    void setDevicePath(const QString &devicePath, const QString &mountPoint);
    void setProgress(int percent);
    void setErrorCode(const QString &errorCode);  // 设置错误码
    void setState(RepairState state);  // 切换对话框状态
    void startSimulatedProgress();  // 启动模拟进度
    RepairState getState() const { return m_state; }

    QString devicePath() const { return m_devicePath; }
    QString mountPoint() const { return m_mountPoint; }

    void reject() override;

private:
    void initConfirmUI();
    void initRepairingUI();
    void initSuccessUI();
    void initFailedUI();
    void updateSimulatedProgress();  // 更新模拟进度

    // 缓动函数：easeOutQuad，增长越来越慢
    double easeOutQuad(double t) { return t * (2 - t); }

    QString m_deviceName;
    QString m_deviceSize;
    QString m_fsType;
    QString m_devicePath;        // Device DBus path
    QString m_mountPoint;        // Device mount point
    QString m_errorCode;         // 错误码
    RepairState m_state { kConfirm };

    DTK_WIDGET_NAMESPACE::DWaterProgress *m_progressWidget { nullptr };
    QTimer *m_progressTimer { nullptr };
    qint64 m_progressStartTime { 0 };  // 进度开始时间
    static constexpr int kMaxProgress = 99;  // 最大进度99%
};

#endif   // REPAIRDIALOG_H
