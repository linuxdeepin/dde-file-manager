/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "desktopinfo.h"

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
#include "drootfilemanager.h"
#include "accessible/accessiblelist.h"
#include "dfmapplication.h"
#include "views/dfilemanagerwindow.h"

#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QProcess>
#include <QLocalSocket>
#include <QPixmapCache>
#include <QSurfaceFormat>

#include <pwd.h>
#include <DApplicationSettings>
#include <QtConcurrent>
#include <QMediaPlayer>
#include <QMediaMetaData>

#ifdef ENABLE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

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
    qCritical() << "break with !SIGTERM! " << sig;

    if (qApp) {
        qApp->quit();
    }
}

void handleSIGPIPE(int sig)
{
    qCritical() << "ignore !SIGPIPE! " << sig;
}

// Within an SSH session, I can use gvfs-mount provided that
// dbus-daemon is launched first and the environment variable DBUS_SESSION_BUS_ADDRESS is set.

// gvfs-mount and other GVFS utilities must all talk to the same D-Bus session. Hence,
// if you use multiple SSH sessions or otherwise use mounts across login sessions, you must:
// - start D-Bus the first time it is needed, at the latest;
// - take care not to let D-Bus end with the session, as long as there are mounted GVFS filesystems;
// - reuse the existing D-Bus session at login time if there is one.
// see https://unix.stackexchange.com/questions/44317/reuse-d-bus-sessions-across-login-sessions for that.
void handleEnvOfOpenAsAdmin()
{
    QProcess p;
    p.start("bash", QStringList() << "-c"
            << "echo $(dbus-launch)");
    p.waitForFinished();
    QString envName("DBUS_SESSION_BUS_ADDRESS");
    QString output(p.readAllStandardOutput());
    QStringList group(output.split(" "));
    for (const QString &vals : group) {
        const QStringList &envGroup = vals.split(",");
        for (const QString &env : envGroup) {
            int mid = env.indexOf("=");
            if (env.startsWith(envName) && mid >= envName.size()) {
                QString v(env.mid(mid + 1));
                QByteArray nameBytes(envName.toLocal8Bit());
                QByteArray vBytes(v.toLocal8Bit());
                int ret = setenv(nameBytes.data(), vBytes.data(), 1);
                qDebug() << "set " << env << "=" << v << "ret=" << ret;
                return;
            }
        }
    }
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
#ifdef ENABLE_JEMALLOC
    // fix bug 89285
    // 设置background_thread=true，让jemalloc后台回收脏数据
    bool oldp = false;
    bool newp = true;
    size_t len = sizeof(bool);
    mallctl("background_thread", &oldp, &len, &newp, len);
#endif

    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")){
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    winId_mtx.first = false;
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    if (qEnvironmentVariableIsSet("PKEXEC_UID")) {
        const quint32 pkexecUID = qgetenv("PKEXEC_UID").toUInt();
        DApplication::customQtThemeConfigPathByUserHome(getpwuid(pkexecUID)->pw_dir);
    }

    // fix "Error mounting location: volume doesn't implement mount” when ope as admin (bug-42653)
    if (DFMGlobal::isRootUser()) {
        handleEnvOfOpenAsAdmin();
    }

    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    if (DFMGlobal::isWayLand()) {
        //以下代码用于视频预览使用
        setenv("PULSE_PROP_media.role", "video", 1);
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
    }

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationName(QMAKE_TARGET);
    app.loadTranslator();
    app.setApplicationDisplayName(app.translate("Application", "File Manager"));
    app.setApplicationVersion(DApplication::buildVersion((QMAKE_VERSION)));
    app.setProductIcon(QIcon::fromTheme("dde-file-manager"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    app.setApplicationDescription(app.translate("Application", "File Manager is a powerful and "
                                                "easy-to-use file management tool, "
                                                "featured with searching, copying, "
                                                "trash, compression/decompression, file property "
                                                "and other useful functions."));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 集成测试标签
#ifdef ENABLE_ACCESSIBILITY
    QAccessible::installFactory(accessibleFactory);
    QAccessible::setActive(true);
#else
    QAccessible::setActive(false);
#endif

    DApplicationSettings setting;

    DFMGlobal::installTranslator();

    LogUtil::registerLogger();
    CommandLineManager::instance()->process();

    //使用异步加载win相关的插件
    auto windPluginLoader = QtConcurrent::run([]() {
        if (CommandLineManager::instance()->isSet("h") || CommandLineManager::instance()->isSet("v"))
            return;
        winId_mtx.second.lock();
        if (winId_mtx.first) {
            winId_mtx.second.unlock();
            /* 针对bug47144音乐预览加载缓慢的问题，在初始化的时候调用QMediaPlayer::hasSupport，下次调用就快很多*/
            QMediaPlayer::hasSupport("application/octet-stream");
            return;
        }
        QWidget *w = new QWidget;
        w->setWindowIcon(QIcon::fromTheme("dde-file-manager"));
        w->winId();
        delete w;
        winId_mtx.second.unlock();
        /* 针对bug47144音乐预览加载缓慢的问题，在初始化的时候调用QMediaPlayer::hasSupport，下次调用就快很多*/
        QMediaPlayer::hasSupport("application/octet-stream");
    });
    // init application object
    DFMApplication fmApp;
    Q_UNUSED(fmApp)

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(static_cast<int>(20 * 1024 * app.devicePixelRatio()));

    // working dir
    if (CommandLineManager::instance()->isSet("w")) {
        QDir::setCurrent(CommandLineManager::instance()->value("w"));
    }

    // open as root
    if (CommandLineManager::instance()->isSet("r")) {
        if (DFMGlobal::isWayLand()) {
            QString cmd = "xhost";
            QStringList args;
            args << "+";
            QProcess p;
            p.start(cmd, args);
            p.waitForFinished();
        }

        QStringList args = app.arguments().mid(1);
        args.removeAll(QStringLiteral("-r"));
        args.removeAll(QStringLiteral("--root"));
        args.removeAll(QStringLiteral("-w"));
        args.removeAll(QStringLiteral("--working-dir"));
        QProcess::startDetached("dde-file-manager-pkexec", args, QDir::currentPath());

        windPluginLoader.waitForFinished();
        return 0;
    }

    if (CommandLineManager::instance()->isSet("h") || CommandLineManager::instance()->isSet("v")) {
        return app.exec();
    }

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = true;
    // cannot open the filemanager when multiple users as an administrator
    // to open the filemanager(bug-42832). therefore, we have to give up single application mode.
    if (DFMGlobal::isOpenAsAdmin()) {
        qDebug() << "oepn as admin";
        isSingleInstance = true;
    } else {
        isSingleInstance = app.setSingleInstance(uniqueKey);
    }

    if (isSingleInstance) {
        // init app
        Q_UNUSED(FileManagerApp::instance())

        DFMGlobal::setInitAppOver();
        //!开始扫描文件，提升启动速度.
        DRootFileManager::instance()->startQuryRootFile();

        if (CommandLineManager::instance()->isSet("d")) {
            fileManagerApp;
            app.setQuitOnLastWindowClosed(false);
        } else {
            CommandLineManager::instance()->processCommand();
        }

        signal(SIGTERM, handleSIGTERM);
        signal(SIGPIPE, handleSIGPIPE); // SIGPIPE 引起程序挂死

#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();

        return request;
#else
        int ret = app.exec();
#ifdef ENABLE_DAEMON
        if (!DFMGlobal::isOpenAsAdmin()) {
            app.closeServer();
            QProcess::startDetached(QString("%1 -d").arg(QString(argv[0])));
        }
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

        QLocalSocket *socket = SingleApplication::newClientProcess(uniqueKey, data);
        if (is_set_get_monitor_files && socket->error() == QLocalSocket::UnknownSocketError) {
            socket->waitForReadyRead();

            for (const QByteArray &i : socket->readAll().split(' '))
                qDebug() << QString::fromLocal8Bit(QByteArray::fromBase64(i));
        }
        windPluginLoader.waitForFinished();
        return 0;
    }
}
