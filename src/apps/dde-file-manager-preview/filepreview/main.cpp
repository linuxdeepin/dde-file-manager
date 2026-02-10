// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewsingleapplication.h"

#include <dfm-base/utils/windowutils.h>

#include <QProcessEnvironment>

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

int main(int argc, char *argv[])
{
    // 管理员模式可能丢失 QT_QPA_PLATFORM
    if (DFMBASE_NAMESPACE::WindowUtils::isX11() && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "dxcb");
    }

    // singlentan process
    PreviewSingleApplication app(argc, argv);

    app.setLayoutDirection(Qt::LeftToRight);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setApplicationName("Deepin File Preview");
    app.setApplicationVersion("v1.0");

    {
        // load translation
        auto appName = app.applicationName();
        app.setApplicationName("dde-file-manager");
        app.loadTranslator();
        app.setApplicationName(appName);
    }

    QString uniqueKey(app.applicationName());
    bool isSinglentanApp = app.setSingleInstance(uniqueKey);

    if (isSinglentanApp) {
        fmInfo() << "main: File preview application started as primary instance, version:" << app.applicationVersion();
        PreviewSingleApplication::processArgs(app.arguments());
        int ret = app.exec();
        fmInfo() << "main: File preview application exiting with code:" << ret;
        return ret;
    } else {
        fmDebug() << "main: Detected existing preview instance, forwarding to primary instance";
        app.handleNewClient(uniqueKey);
        return 0;
    }
}
