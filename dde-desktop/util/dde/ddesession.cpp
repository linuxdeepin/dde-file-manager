#include "ddesession.h"

#include <QDBusInterface>
#include <QDBusPendingCall>

void Dde::Session::RegisterDdeSession()
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    QByteArray cookie = qgetenv(envName);
    qunsetenv(envName);

    if (!cookie.isEmpty()) {
        QDBusInterface iface("com.deepin.SessionManager",
                             "/com/deepin/SessionManager",
                             "com.deepin.SessionManager",
                             QDBusConnection::sessionBus());
        iface.asyncCall("Register", QString(cookie));
    }
}
