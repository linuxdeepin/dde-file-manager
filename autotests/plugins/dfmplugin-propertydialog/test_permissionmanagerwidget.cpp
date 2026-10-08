// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS 137409: 属性窗口权限页 selectFileUrl/updateFileUrl 切换文件后未同步 selectUrl，
// 导致勾选"允许以程序身份执行"等权限变更被应用到旧文件上。
// 回归点：onComboBoxChanged 触发的 sendSetPermissionManager 必须携带最新选中的文件 URL。

#include "stubext.h"

#include "views/permissionmanagerwidget.h"
#include "events/propertyeventcall.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QTest>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

namespace {
void writeFile137409(const QString &path, const QByteArray &content)
{
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(content);
    f.close();
}
}   // namespace

class PermissionManagerWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        ASSERT_TRUE(tempDir.isValid());
        widget = new PermissionManagerWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    PermissionManagerWidget *widget = nullptr;
};

// PMS:137409 修复前 selectFileUrl(QUrl()) 会崩溃/污染内部状态；修复后空 URL 应被安全忽略
TEST_F(PermissionManagerWidgetTest, BUG137409_SelectFileUrl_EmptyUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->selectFileUrl(QUrl()));

    // 空 URL 后再触发权限变更信号，不应崩溃
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(widget, "onComboBoxChanged"));
    SUCCEED();
}

// PMS:137409 连续选择两个文件后触发权限变更，事件必须携带最后选择的文件 B（修复前携带旧文件/空 URL）
TEST_F(PermissionManagerWidgetTest, BUG137409_SelectFileUrl_ChangesTargetOfPermissionChange)
{
    const QString fileA = tempDir.filePath("perm-a.sh");
    const QString fileB = tempDir.filePath("perm-b.sh");
    writeFile137409(fileA, "#!/bin/sh\necho A\n");
    writeFile137409(fileB, "#!/bin/sh\necho B\n");

    widget->selectFileUrl(QUrl::fromLocalFile(fileA));
    widget->selectFileUrl(QUrl::fromLocalFile(fileB));

    QUrl capturedUrl;
    stub.set_lamda(&PropertyEventCall::sendSetPermissionManager,
                   [&capturedUrl](quint64, const QUrl &url, const QFileDevice::Permissions) {
                       capturedUrl = url;
                   });

    // 私有槽 onComboBoxChanged：模拟用户修改权限下拉框
    QMetaObject::invokeMethod(widget, "onComboBoxChanged");

    EXPECT_EQ(capturedUrl, QUrl::fromLocalFile(fileB));
}

// PMS:137409 updateFileUrl 切换目标文件后，权限变更事件必须携带新文件 C（修复前沿用旧 selectUrl）
TEST_F(PermissionManagerWidgetTest, BUG137409_UpdateFileUrl_ChangesTargetOfPermissionChange)
{
    const QString fileA = tempDir.filePath("perm-old.sh");
    const QString fileC = tempDir.filePath("perm-new.sh");
    writeFile137409(fileA, "#!/bin/sh\necho old\n");
    writeFile137409(fileC, "#!/bin/sh\necho new\n");

    widget->selectFileUrl(QUrl::fromLocalFile(fileA));
    widget->updateFileUrl(QUrl::fromLocalFile(fileC));

    QUrl capturedUrl;
    stub.set_lamda(&PropertyEventCall::sendSetPermissionManager,
                   [&capturedUrl](quint64, const QUrl &url, const QFileDevice::Permissions) {
                       capturedUrl = url;
                   });

    QMetaObject::invokeMethod(widget, "onComboBoxChanged");

    EXPECT_EQ(capturedUrl, QUrl::fromLocalFile(fileC));
}
