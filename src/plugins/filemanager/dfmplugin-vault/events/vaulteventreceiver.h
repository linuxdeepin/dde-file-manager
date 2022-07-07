#ifndef VAULTEVENTRECEIVER_H
#define VAULTEVENTRECEIVER_H

#include "dfmplugin_vault_global.h"

#include <QObject>
namespace dfmplugin_vault {
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
    void handleCurrentUrlChanged(const quint64 &winId, const QUrl &url);
};
}
#endif   // VAULTEVENTRECEIVER_H
