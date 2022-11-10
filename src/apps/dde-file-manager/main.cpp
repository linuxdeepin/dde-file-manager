/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#include "config.h"   //cmake
#include "singleapplication.h"
#include "commandparser.h"

#include "dfm-base/utils/sysinfoutils.h"

#include <dfm-framework/dpf.h>

#include <DApplicationSettings>
#include <DSysInfo>
#include <QIcon>
#include <QDir>
#include <QTextCodec>
#include <QProcess>

#include <signal.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

#define BUILD_VERSION ((QString(VERSION) == "") ? "6.0.0.0" : QString(VERSION))

// defualt plugin IID
static constexpr char kFmPluginInterface[] { "org.deepin.plugin.filemanager" };
static constexpr char kCommonPluginInterface[] { "org.deepin.plugin.common" };
static constexpr char kPluginCore[] { "dfmplugin-core" };
static constexpr char kLibCore[] { "libdfmplugin-core.so" };

/* Within an SSH session, I can use gvfs-mount provided that
 * dbus-daemon is launched first and the environment variable DBUS_SESSION_BUS_ADDRESS is set.
 *
 * gvfs-mount and other GVFS utilities must all talk to the same D-Bus session. Hence,
 * if you use multiple SSH sessions or otherwise use mounts across login sessions, you must:
 * - start D-Bus the first time it is needed, at the latest;
 * - take care not to let D-Bus end with the session, as long as there are mounted GVFS filesystems;
 * - reuse the existing D-Bus session at login time if there is one.
 * see https://unix.stackexchange.com/questions/44317/reuse-d-bus-sessions-across-login-sessions for that.
 */
static void setEnvForRoot()
{
    QProcess p;
    p.start("bash", QStringList() << "-c"
                                  << "echo $(dbus-launch --autolaunch $(cat /var/lib/dbus/machine-id))");
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

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    const QString &&pluginsDir { qApp->applicationDirPath() + "/../../plugins" };
    QStringList pluginsDirs;
    if (!QDir(pluginsDir).exists()) {
        qInfo() << QString("Path does not exist, use path : %1").arg(DFM_PLUGIN_COMMON_CORE_DIR);
        pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                    << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                    << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                    << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
    } else {
        pluginsDirs.push_back(pluginsDir + "/filemanager");
        pluginsDirs.push_back(pluginsDir + "/common");
        pluginsDirs.push_back(pluginsDir);
    }

    qDebug() << "using plugins dir:" << pluginsDirs;
    if (DTK_CORE_NAMESPACE::DSysInfo::isCommunityEdition())   // TODO(zhangs): use config
        DPF_NAMESPACE::LifeCycle::initialize({ kFmPluginInterface, kCommonPluginInterface }, pluginsDirs, { "dfmplugin-vault" });
    else
        DPF_NAMESPACE::LifeCycle::initialize({ kFmPluginInterface, kCommonPluginInterface }, pluginsDirs);

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore))
        return false;
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // start filemanager, must called it after core plugin loaded
    CommandParser::instance().bindEvents();

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
        return false;

    dpfCheckTimeEnd();

    return true;
}

static void handleSIGTERM(int sig)
{
    qCritical() << "break with !SIGTERM! " << sig;

    if (qApp) {
        qApp->quit();
    }
}

static void handleSIGPIPE(int sig)
{
    qCritical() << "ignore !SIGPIPE! " << sig;
}

static void initEnv()
{
    // for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    if (qEnvironmentVariable("CLUTTER_IM_MODULE") == QStringLiteral("fcitx")) {
        setenv("QT_IM_MODULE", "fcitx", 1);
    }

    if (SysInfoUtils::isOpenAsAdmin())
        setEnvForRoot();
}

static void initLog()
{
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

int main(int argc, char *argv[])
{
    initEnv();

    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    SingleApplication a(argc, argv);

    a.setOrganizationName(ORGANIZATION_NAME);
    a.loadTranslator();
    a.setApplicationDisplayName(a.translate("Application", "File Manager"));
    a.setApplicationVersion(BUILD_VERSION);
    a.setProductIcon(QIcon::fromTheme("dde-file-manager"));
    a.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    a.setApplicationDescription(a.translate("Application", "File Manager is a powerful and "
                                                           "easy-to-use file management tool, "
                                                           "featured with searching, copying, "
                                                           "trash, compression/decompression, file property "
                                                           "and other useful functions."));
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    DApplicationSettings setting;

    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    initLog();

    CommandParser::instance().process();

    // working dir
    if (CommandParser::instance().isSet("w")) {
        QDir::setCurrent(CommandParser::instance().value("w"));
    }

    // open as root
    if (CommandParser::instance().isSet("r")) {
        qInfo() << "Open as admin";
        a.openAsAdmin();
        return 0;
    }

    if (CommandParser::instance().isSet("h") || CommandParser::instance().isSet("v")) {
        return a.exec();
    }

    const QString &uniqueKey = a.applicationName();

    bool isSingleInstance = true;

    if (!SysInfoUtils::isOpenAsAdmin())
        isSingleInstance = a.setSingleInstance(uniqueKey);

    if (isSingleInstance) {
        if (!pluginsLoad()) {
            qCritical() << "Load pugin failed!";
            abort();
        }
        signal(SIGTERM, handleSIGTERM);
        signal(SIGPIPE, handleSIGPIPE);
    } else {
        qInfo() << "new client";
        a.handleNewClient(uniqueKey);
        return 0;
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
