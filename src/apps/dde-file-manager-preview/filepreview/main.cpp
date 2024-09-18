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

    app.setOrganizationName(ORGANIZATION_NAME);
    app.setApplicationName("Deepin File Preview");
    app.setApplicationVersion("v1.0");

    PreviewSingleApplication::processArgs(app.arguments());
    return app.exec();
}