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
    void computerOpenItem(const QUrl &url);
};
DPVAULT_END_NAMESPACE
#endif   // VAULTEVENTRECEIVER_H
