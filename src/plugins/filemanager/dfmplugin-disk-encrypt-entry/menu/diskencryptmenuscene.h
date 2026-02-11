// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTMENUSCENE_H
#define DISKENCRYPTMENUSCENE_H

#include "gui/encryptparamsinputdialog.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <dfm-mount/dmount.h>

#include <QUrl>
#include <QDBusInterface>

class QAction;

namespace dfmplugin_diskenc {

class DiskEncryptMenuCreator : public dfmbase::AbstractSceneCreator
{
    Q_OBJECT
    // AbstractSceneCreator interface
public:
    virtual dfmbase::AbstractMenuScene *create() override;
    static inline QString name()
    {
        return "DiskEncryptMenu";
    }
};

class DiskEncryptMenuScene : public dfmbase::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit DiskEncryptMenuScene(QObject *parent = nullptr);

    // AbstractMenuScene interface
public:
    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual void updateState(QMenu *parent) override;

    static void doReencryptDevice(const disk_encrypt::DeviceEncryptParam &param);

protected:
    static void encryptDevice(const disk_encrypt::DeviceEncryptParam &param);
    static void decryptDevice(const disk_encrypt::DeviceEncryptParam &param);
    static void changePassphrase(disk_encrypt::DeviceEncryptParam param);
    static void unlockDevice(const QString &dev);

    static void doEncryptDevice(const disk_encrypt::DeviceEncryptParam &param);
    static void doDecryptDevice(const disk_encrypt::DeviceEncryptParam &param);
    static void doChangePassphrase(const disk_encrypt::DeviceEncryptParam &param);

    static QString generateTPMConfig();
    static QString generateTPMToken(const QString &device, bool pin);
    static QString getBase64Of(const QString &fileName);

    // Send credentials via file descriptor for secure D-Bus transmission
    static bool sendCredentialsViaFd(QDBusInterface &iface, const QString &method,
                                     const QVariantMap &params, bool asyncCall = false);

    static void onUnlocked(bool ok, dfmmount::OperationErrorInfo, QString);
    static void onMounted(bool ok, dfmmount::OperationErrorInfo, QString);

    static void unmountBefore(const std::function<void(const disk_encrypt::DeviceEncryptParam &)> &after, const disk_encrypt::DeviceEncryptParam &param);
    enum OpType { kUnmount,
                  kLock };
    static void onUnmountError(OpType t, const QString &dev, const dfmmount::OperationErrorInfo &err);

    void sortActions(QMenu *parent);
    void updateActions();

private:
    QMap<QString, QAction *> actions;
    bool hasCryptHeader { false };
    QVariantHash selectedItemInfo;
    disk_encrypt::DeviceEncryptParam param;
};

}

#endif   // DISKENCRYPTMENUSCENE_H
