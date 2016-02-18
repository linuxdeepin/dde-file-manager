#ifndef DEFINE_H
#define DEFINE_H


#include "widgets/singleton.h"
#include "widgets/commandlinemanager.h"
#include "global.h"

#include <DLog>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>

DUTIL_USE_NAMESPACE

#undef signals
extern "C" {
  #include <gtk/gtk.h>
}
#define signals public

static void requrestUpdateIcons()
{
    GtkIconTheme* gs = gtk_icon_theme_get_default();
    auto a = gtk_icon_theme_get_example_icon_name(gs);
    if (a != NULL) g_free(a);
    //can not passing QObject to the callback function,so use signal
    emit signalManager->gtkIconThemeChanged();
}

void initGtkThemeWatcher()
{
    GtkIconTheme* gs = gtk_icon_theme_get_default();
    g_signal_connect(gs, "changed",
                     G_CALLBACK(requrestUpdateIcons), NULL);
    auto a = gtk_icon_theme_get_example_icon_name(gs);
    if (a != NULL) g_free(a);
}

void RegisterLogger(){
    bool isSet =  CommandLineManager::instance()->isSet("logDestination");
    QString value = CommandLineManager::instance()->value("logDestination");
    if (isSet){
        if (value == "stdout"){
            DLogManager::registerConsoleAppender();
        }else if (value == "file"){
            DLogManager::registerFileAppender();
        }else{
            DLogManager::registerFileAppender();
        }
    }else{
#if !defined(QT_NO_DEBUG)
        DLogManager::registerConsoleAppender();
#endif
        DLogManager::registerFileAppender();
    }
}

// let startdde know that we've already started.
void RegisterDdeSession()
{
    char envName[] = "DDE_SESSION_PROCESS_COOKIE_ID";

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

#endif // DEFINE_H

