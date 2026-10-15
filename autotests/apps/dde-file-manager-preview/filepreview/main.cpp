// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Custom main for ut-filepreview: PreviewSingleApplication itself is THE
// application object so its constructor / connections / destructor are
// exercised for real, and XDG_RUNTIME_DIR points into a sandbox so the local
// server socket never touches the developer's runtime directory.

#include <gtest/gtest.h>

#include "previewsingleapplication.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

int main(int argc, char **argv)
{
    static QTemporaryDir sandbox;
    const QString runtime = sandbox.path() + "/runtime";
    QDir().mkpath(runtime);
    // QStandardPaths warns unless the runtime dir is owner-only
    QFile::setPermissions(runtime, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                       | QFileDevice::ExeOwner);
    qputenv("XDG_RUNTIME_DIR", runtime.toUtf8());

    PreviewSingleApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    return result;
}
