// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewsingleapplication.h"

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

int main(int argc, char *argv[])
{
    // singlentan process
    PreviewSingleApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setApplicationName("Deepin File Preview");
    app.setApplicationVersion("v1.0");

    QString uniqueKey(app.applicationName());
    bool isSinglentanApp = app.setSingleInstance(uniqueKey);

    if (isSinglentanApp) {
        PreviewSingleApplication::processArgs(app.arguments());
        return app.exec();
    } else {
        app.handleNewClient(uniqueKey);
        return 0;
    }
}
