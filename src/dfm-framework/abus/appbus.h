#ifndef APPBUS_H
#define APPBUS_H

#include "dfm-framework/dfm_framework_global.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class AppBusPrivate;
class AppBus : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AppBus)
    AppBusPrivate * const d;

public:
    explicit AppBus(QObject *parent = nullptr);

    virtual ~AppBus();

    QStringList onlineServer();

    QString mimeServer();

    bool isMimeServer(const QString& serverName);

Q_SIGNALS:
    void newCreateAppBus(const QString& serverName);

};

DPF_END_NAMESPACE

#endif // APPBUS_H
