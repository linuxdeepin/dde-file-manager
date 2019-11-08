/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <DApplication>
#include <QDesktopWidget>
#include <DLog>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QDBusMetaType>
#include "app/cmdmanager.h"
#include "view/mainwindow.h"
#include "dplatformwindowhandle.h"
#include "dialogs/messagedialog.h"
#include "app/singletonapp.h"
#include "utils/udisksutils.h"
#include <QProcessEnvironment>
#include <QX11Info>
#include <X11/Xlib.h>
DCORE_USE_NAMESPACE
//DWIDGET_BEGIN_NAMESPACE

#include <pwd.h>

int main(int argc, char *argv[])
{
    //Logger
    DLogManager::registerConsoleAppender();

    qDBusRegisterMetaType<QPair<bool,QString>>();
    qDBusRegisterMetaType<QByteArrayList>();

    //Init Environment
    qputenv("QT_IM_MODULE", "xim");
    qputenv("GTK_IM_MODULE", "xim");
    qputenv("QT4_IM_MODULE", "xim");
    qputenv("IM_CONFIG_PHASE", "1");
    qputenv("XMODIFIERS", "@im");
    qputenv("IBUS_DISABLE_SNOOPER", "1");
    qputenv("QT_LINUX_ACCESSIBILITY_ALWAYS_ON", "1");
    qputenv("WINDOWPATH","2");
    qputenv("XDG_SESSION_TYPE","x11");
    qputenv("PAGER", "less");
    qputenv("PAGER", "-R");
    qputenv("SHLVL", "1");
    qputenv("DBUS_STARTER_BUS_TYPE", "session");
    qputenv("GDMSESSION", "deepin");
    qputenv("DESKTOP_SESSION", "deepin");

    if (qEnvironmentVariableIsSet("PKEXEC_UID")) {
        const quint32 pkexecUID = qgetenv("PKEXEC_UID").toUInt();
        DApplication::customQtThemeConfigPathByUserHome(getpwuid(pkexecUID)->pw_dir);
    }

    //Load DXcbPlugin
    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);

    //Singleton app handle
    bool isSingletonApp = SingletonApp::instance()->setSingletonApplication("usb-device-formatter");
    if(!isSingletonApp)
        return 0;

    //Load translation
    QTranslator *translator = new QTranslator(QCoreApplication::instance());

    translator->load("/usr/share/usb-device-formatter/translations/usb-device-formatter_"
                     +QLocale::system().name()+".qm");
    a.installTranslator(translator);

    a.setOrganizationName("deepin");
    a.setApplicationName("Deepin usb device formatter");
    a.setApplicationVersion("1.0");
    a.setWindowIcon(QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager")));
    a.setQuitOnLastWindowClosed(true);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    //Command line
    CMDManager::instance()->process(a);

    // Check if we need display help text.
    if (CMDManager::instance()->positionalArguments().isEmpty()) {
        CMDManager::instance()->showHelp();
    }

    //Check if exists path
    const QString path = CMDManager::instance()->getPath();
    if (path.isEmpty() || !path.startsWith("/dev/") || !QFile::exists(path)) {
        QString message = QObject::tr("Device does not exist");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    //Check if the device is read-only
    UDisksBlock blk(path);
    if (blk.isReadOnly()){
        QString message = QObject::tr("The device is read-only");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    MainWindow* w = new MainWindow(path);
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    w->move(rect.x(), rect.y());

    if(CMDManager::instance()->isSet("m")){
        int parentWinId = CMDManager::instance()->getWinId();
        int winId = w->winId();

        if(parentWinId != -1)
            qDebug() << XSetTransientForHint(QX11Info::display(), (Window)winId, (Window)parentWinId);
    }

    int code = a.exec();
    quick_exit(code);
}
