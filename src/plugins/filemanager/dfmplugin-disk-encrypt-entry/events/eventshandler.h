// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EVENTSHANDLER_H
#define EVENTSHANDLER_H

#include <QObject>
#include <QMap>
#include <QPointer>

namespace dfmplugin_diskenc {
class EncryptProgressDialog;
class EncryptParamsInputDialog;
class EventsHandler : public QObject
{
    Q_OBJECT
public:
    static EventsHandler *instance();
    void bindDaemonSignals();
    void hookEvents();
    bool isTaskWorking();
    bool hasPendingTask();
    QString unfinishedDecryptJob();
    bool isUnderOperating(const QString &device);
    int deviceEncryptStatus(const QString &device);
    void resumeEncrypt(const QString &device);
    QString holderDevice(const QString &device);
    bool onAcquireDevicePwd(const QString &dev, QString *pwd, bool *giveup);
    void setAutoStartDFM(bool enable);

private Q_SLOTS:
    void onEncryptProgress(const QString &, const QString &, double);
    void onDecryptProgress(const QString &, const QString &, double);
    void onInitEncryptFinished(const QVariantMap &);
    void onEncryptFinished(const QVariantMap &);
    void onDecryptFinished(const QVariantMap &);
    void onChgPwdFinished(const QVariantMap &);
    void onRequestAuthArgs(const QVariantMap &);
    void onOverlayDMModeChanged(bool enabled, int result);

    void ignoreParamRequest();

    QString acquirePassphrase(const QString &dev, bool &cancelled);
    QString acquirePassphraseByPIN(const QString &dev, bool &cancelled);
    QString acquirePassphraseByTPM(const QString &dev, bool &cancelled);
    QString acquirePassphraseByRec(const QString &dev, bool &cancelled);

    void showPreEncryptError(const QString &device, const QString &devName, int code);
    void showDecryptError(const QString &device, const QString &devName, int code);
    void showChgPwdError(const QString &device, const QString &devName, int code);

    void requestReboot();
    bool canUnlock(const QString &device);

private:
    explicit EventsHandler(QObject *parent = nullptr);

    QMap<QString, QPointer<EncryptProgressDialog>> encryptDialogs;
    QMap<QString, QPointer<EncryptProgressDialog>> decryptDialogs;
    QMap<QString, QPointer<EncryptParamsInputDialog>> encryptInputs;
signals:
};
}
#endif   // EVENTSHANDLER_H
