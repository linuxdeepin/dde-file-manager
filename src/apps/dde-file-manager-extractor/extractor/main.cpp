// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extractorapp.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <type_traits>

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/utils/loggerrules.h>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(EXTRACTOR_PLUGIN_NAMESPACE)
EXTRACTOR_PLUGIN_END_NAMESPACE

// debug:
// 1. sudo sysctl kernel.yama.ptrace_scope=0
// 2. attach: gdb -p $(PID)

static void initLogFilter()
{
    DFMBASE_NAMESPACE::LoggerRules::instance().initLoggerRules();
}

int main(int argc, char *argv[])
{
    initLogFilter();
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dde-file-manager-extractor");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("DDE File Manager Content Extractor");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption pluginPathOption(
            "plugin-path",
            QCoreApplication::translate("main", "Path to the plugin directory"),
            QCoreApplication::translate("main", "path"));

    parser.addOption(pluginPathOption);
    parser.process(app);

    // Get plugin path
    QString pluginPath = parser.value(pluginPathOption);
    if (pluginPath.isEmpty()) {
        auto defaultPath = DFM_PLUGIN_EXTRACTOR_DIR;
        static_assert(std::is_same<decltype(defaultPath), const char *>::value,
                      "DFM_PLUGIN_EXTRACTOR_DIR is not a string.");

#ifdef QT_DEBUG
        const QString debugPluginPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../plugins");
        if (QDir(debugPluginPath).exists()) {
            pluginPath = debugPluginPath;
        } else {
            pluginPath = QString::fromLocal8Bit(defaultPath);
        }
#else
        pluginPath = QString::fromLocal8Bit(defaultPath);
#endif
    }

    // Create and initialize extractor application
    extractor_plugin::ExtractorApp extractor;
    if (!extractor.initialize(pluginPath)) {
        qCritical() << "Failed to initialize extractor with plugin path:" << pluginPath;
        return 1;
    }

    // Run the main loop
    extractor.run();

    return 0;
}
