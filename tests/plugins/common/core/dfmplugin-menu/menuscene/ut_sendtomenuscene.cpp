// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-menu/menuscene/sendtomenuscene.h"
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QMenu>
#include <QDir>
#include <QUrl>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPMENU_USE_NAMESPACE

class UT_SendToMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);}
    virtual void TearDown() override { stub.clear(); }
    ~UT_SendToMenuScene() override;

private:
    stub_ext::StubExt stub;
};

UT_SendToMenuScene::~UT_SendToMenuScene(){

}

TEST_F(UT_SendToMenuScene, bug_203001_initialize)
{
    QVariantHash params;
    auto url = QUrl::fromLocalFile(QDir::currentPath());
    params[MenuParamKey::kCurrentDir] = url;
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;
    SendToMenuScene scene;
    EXPECT_TRUE(scene.initialize(params));

    auto focusInfo = InfoFactory::create<FileInfo>(url);
    params[MenuParamKey::kIsEmptyArea] = false;
    EXPECT_FALSE(scene.initialize(params));

    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << url);
    EXPECT_TRUE(scene.initialize(params));
}
