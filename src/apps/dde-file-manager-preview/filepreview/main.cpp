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

#ifdef QT_DEBUG
    // 未安装的 debug 构建中 DConfig 的 appId 为空，导致无法读取系统主题（DTK 偏好）。
    // 切换到文件后端绕过此限制，使浅色/深色主题能正确跟随系统。
    if (qEnvironmentVariableIsEmpty("DSG_DCONFIG_BACKEND_TYPE")) {
        qputenv("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
    }
#endif

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
