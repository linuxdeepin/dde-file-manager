#ifndef SMBINTEGRATIONMANAGER_H
#define SMBINTEGRATIONMANAGER_H

#include <QObject>
#include "dfmplugin_smbbrowser_global.h"
#include "dfm-base/base/application/application.h"

#undef signals
extern "C" {
#include <libsecret/secret.h>
}
#define signals public

DFMBASE_USE_NAMESPACE
Q_DECLARE_METATYPE(QList<QUrl> *)

namespace dfmplugin_smbbrowser {

class SmbIntegrationManager : public QObject
{
    Q_OBJECT
public:
    static SmbIntegrationManager *instance();
    bool isSmbIntegrationEnabled();
    bool isStashMountsEnabled();
    void removeStashedIntegrationFromConfig(const QUrl &url);
    void removeStashedSeperatedItem(const QUrl &url);
    void handleProtocolDeviceUnmounted(quint64 windowId, const QVariantHash &umountedSmbData);
    void umountAllProtocolDevice(quint64 windowId, const QUrl &entryUrl, bool forgetPassword);
    bool isSmbShareDirMounted(const QUrl &url);
    QStringList getSmbMountPathsByUrl(const QUrl &url);
    void actUmount(quint64 winId, const QStringList &smbMountPaths);
    void addSmbIntegrationItem(const QUrl &hostUrl, ContextMenuCallback contexMenu);
    void onOpenItem(quint64 winId, const QUrl &url);
    void addSmbIntegrationFromConfig(ContextMenuCallback ctxMenuHandle, bool forSidebar = true);
    void removeSmbIntegrationFromConfig(const QUrl &entryUrl);

    void addStashedSeperatedItemToSidebar(QVariantMap stashedSeperatedData, ContextMenuCallback contexMenu);
    void addStashedSeperatedItemToComputer(const QList<QUrl> &urlList);
    bool handleItemListFilter(QList<QUrl> *items);
    bool handleItemFilterOnAdd(const QUrl &devUrl);
    bool handleItemFilterOnRemove(const QUrl &devUrl);
    void setWindowId(quint64 winId);
    void switchIntegrationMode(bool value);
    void addIntegrationItemToComputer(const QUrl &hostUrl);

public Q_SLOTS:
    void computerOpenItem(quint64 winId, const QUrl &url);

Q_SIGNALS:
    void refreshToSmbIntegrationMode(quint64 winId);
    void refreshToSmbSeperatedMode(const QVariantMap &stashedSeperatedData, const QList<QUrl> &urls);

private:
    explicit SmbIntegrationManager(QObject *parent = nullptr);
    void addIntegrationItemToSidebar(const QUrl &hostUrl, ContextMenuCallback contexMenu);
    void doSwitchToSmbIntegratedMode(const QList<QUrl> &stashedUrls);
    void doSwitchToSmbSeperatedMode(const QVariantMap &stashedSeperatedData, const QList<QUrl> &stashedUrls);
    bool existSmbMount(const QString &host);
    void stashSmbMount(const QString &id);

    void clearPasswd(const QUrl &url);
    QString parseServer(const QString &uri);
    static const SecretSchema *smbSchema();
    static const SecretSchema *ftpSchema();
    static void onPasswdCleared(GObject *obj, GAsyncResult *res, gpointer data);

protected Q_SLOTS:
    void onGenAttributeChanged(Application::GenericAttribute ga, const QVariant &value);
    void onProtocolDeviceUnmounted(const QString &id);

private:
    bool isSmbIntegrated { true };
    bool isSmbStashMountsEnabled { true };
    quint64 windowId { 0 };   // 需要判0
};
}
#endif   // SMBINTEGRATIONMANAGER_H
