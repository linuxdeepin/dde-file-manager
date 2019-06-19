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

#include <signal.h>

#include "durl.h"
#include "dfmglobal.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"

#include "filemanagerapp.h"
#include "logutil.h"
#include "singleapplication.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "commandlinemanager.h"

#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "dialogs/openwithdialog.h"
#include "controllers/appcontroller.h"
#include "singleton.h"
#include "gvfs/gvfsmountmanager.h"

// DBus
#include "views/themeconfig.h"
#include "dfmapplication.h"

#include <dthememanager.h>

#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QProcess>
#include <QLocalSocket>
#include <QPixmapCache>

#include <pwd.h>

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

#define fileManagerApp FileManagerApp::instance()

// blumia: DDE not yet got fully support about session management, so when logout or shutdown,
//         the config file won't save. On mips64el, sw, arm, there will be a "warm-up" process
//         running in the background (dde-file-manager -d) and the file manager instance will
//         not got exit when all visible DFM window got closed, so that means the file manager
//         config file will never got saved.
// blumia: Handling SIGTERM is a workaround way to fix that issue, but we still need to add
//         session management support to DDE.
void handleSIGTERM(int sig)
{
    qDebug() << "!SIGTERM!" << sig;

    if (qApp) {
        qApp->quit();
    }
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    if (qEnvironmentVariableIsSet("PKEXEC_UID")) {
        const quint32 pkexecUID = qgetenv("PKEXEC_UID").toUInt();
        DApplication::customQtThemeConfigPathByUserHome(getpwuid(pkexecUID)->pw_dir);
    }

    SingleApplication::loadDXcbPlugin();
    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationName(QMAKE_TARGET);
    app.loadTranslator();
    app.setApplicationDisplayName(app.translate("Application", "Deepin File Manager"));
    app.setApplicationVersion(DApplication::buildVersion((QMAKE_VERSION)));
    QIcon icon(":/images/images/dde-file-manager_96.svg");
    app.setProductIcon(icon);
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    app.setApplicationDescription(app.translate("Application", "File Manager is a file management tool independently "
                                                               "developed by Deepin Technology, featured with searching, "
                                                               "copying, trash, compression/decompression, file property "
                                                               "and other file management functions."));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef DISABLE_QUIT_ON_LAST_WINDOW_CLOSED
    app.setQuitOnLastWindowClosed(false);
#endif

    DFMGlobal::installTranslator();

    LogUtil::registerLogger();

    // init application object
    DFMApplication fmApp;
    Q_UNUSED(fmApp)

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(20 * 1024 * app.devicePixelRatio());

    CommandLineManager::instance()->process();

    // working dir
    if (CommandLineManager::instance()->isSet("w")) {
        QDir::setCurrent(CommandLineManager::instance()->value("w"));
    }

    // open as root
    if (CommandLineManager::instance()->isSet("r")) {
        QStringList args = app.arguments().mid(1);
        args.removeAll(QStringLiteral("-r"));
        args.removeAll(QStringLiteral("--root"));
        args.removeAll(QStringLiteral("-w"));
        args.removeAll(QStringLiteral("--working-dir"));
        QProcess::startDetached("dde-file-manager-pkexec", args, QDir::currentPath());

        return 0;
    }

    if (CommandLineManager::instance()->isSet("h") || CommandLineManager::instance()->isSet("v")) {
        return app.exec();
    }

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);

    if (isSingleInstance) {
        // init app
        Q_UNUSED(FileManagerApp::instance())

        if (CommandLineManager::instance()->isSet("d")) {
            fileManagerApp;
#ifdef AUTO_RESTART_DEAMON
            QWidget w;
            w.setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
            w.setAttribute(Qt::WA_TranslucentBackground);
            w.resize(0, 0);
            w.show();
#endif
        } else {
            CommandLineManager::instance()->processCommand();
        }

        signal(SIGTERM, handleSIGTERM);

#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();

        return request;
#else
        int ret = app.exec();
#ifdef AUTO_RESTART_DEAMON
        app.closeServer();
        QProcess::startDetached(QString("%1 -d").arg(QString(argv[0])));
#endif
        return ret;
#endif
    } else {
        QByteArray data;
        bool is_set_get_monitor_files = false;

        for (const QString &arg : app.arguments()) {
            if (arg == "--get-monitor-files")
                is_set_get_monitor_files = true;

            if (!arg.startsWith("-") && QFile::exists(arg))
                data.append(QDir(arg).absolutePath().toLocal8Bit().toBase64());
            else
                data.append(arg.toLocal8Bit().toBase64());

            data.append(' ');
        }

        if (!data.isEmpty())
            data.chop(1);

        QWidget w;
        w.setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
        w.setAttribute(Qt::WA_TranslucentBackground);
        w.resize(1, 1);
        w.show();

        QLocalSocket *socket = SingleApplication::newClientProcess(uniqueKey, data);
        if (is_set_get_monitor_files && socket->error() == QLocalSocket::UnknownSocketError) {
            socket->waitForReadyRead();

            for (const QByteArray &i : socket->readAll().split(' '))
                qDebug() << QString::fromLocal8Bit(QByteArray::fromBase64(i));
        }

        return 0;
    }
}
