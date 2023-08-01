// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <DApplication>
#include <DSysInfo>

#include <QDebug>
#include <QDir>

#include <dfm-framework/dpf.h>

#include <signal.h>

static constexpr char kServerInterface[] { "org.deepin.plugin.server" };
static constexpr char kPluginCore[] { "serverplugin-core" };
static constexpr char kLibCore[] { "libserverplugin-core.so" };

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static void initLog()
{
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

static bool isLoadVaultPlugin()
{
    DSysInfo::UosType uosType = DSysInfo::uosType();
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    if (DSysInfo::UosServer == uosType) {
        if (DSysInfo::UosEnterprise == uosEdition
            || DSysInfo::UosEnterpriseC == uosEdition
            || DSysInfo::UosEuler == uosEdition) {
            return true;
        }
    } else if (DSysInfo::UosDesktop == uosType) {
        if (DSysInfo::UosProfessional == uosEdition
            || static_cast<int>(DSysInfo::UosEnterprise) == static_cast<int>(uosEdition + 1)
            || DSysInfo::UosEducation == uosEdition) {
            return true;
        }
    }
    return false;
}

static bool pluginsLoad()
{
    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qInfo() << QString("Load plugins path : %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/server");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_SERVER_EDGE_DIR);
#endif
    qInfo() << "Using plugins dir:" << pluginsDirs;

    QStringList blackNames;
    if (!isLoadVaultPlugin())
        blackNames << "serverplugin-vaultdaemon";

    DPF_NAMESPACE::LifeCycle::initialize({ kServerInterface }, pluginsDirs, blackNames);

    qInfo() << "Depend library paths:" << QCoreApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore)) {
        qWarning() << corePlugin->fileName() << "is not" << kLibCore;
        return false;
    }
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
        return false;

    return true;
}

static void handleSIGTERM(int sig)
{
    qCritical() << "break with !SIGTERM! " << sig;

    if (qApp) {
        qApp->quit();
    }
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    {
        // load translation
        QString appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
    }

    signal(SIGTERM, handleSIGTERM);

    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    initLog();

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
