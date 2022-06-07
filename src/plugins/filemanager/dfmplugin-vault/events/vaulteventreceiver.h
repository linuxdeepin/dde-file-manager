#ifndef VAULTEVENTRECEIVER_H
#define VAULTEVENTRECEIVER_H

#include "dfmplugin_vault_global.h"

#include <QObject>
DPVAULT_BEGIN_NAMESPACE
class VaultEventReceiver : public QObject
{
    Q_OBJECT
private:
    VaultEventReceiver(QObject *parent = nullptr);

public:
    static VaultEventReceiver *instance();

    void connectEvent();

public slots:
    void computerOpenItem(quint64 winId, const QUrl &url);
    bool handleNotAllowedAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl);
};
DPVAULT_END_NAMESPACE
#endif   // VAULTEVENTRECEIVER_H
