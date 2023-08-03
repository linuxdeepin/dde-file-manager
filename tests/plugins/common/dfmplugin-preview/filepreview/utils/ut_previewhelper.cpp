// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"
#include "dfmplugin_filepreview_global.h"
#include "utils/previewhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPFILEPREVIEW_USE_NAMESPACE

TEST(UT_PreviewHelper, IsPreviewEnabled)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::value, []{
        return QVariant::fromValue<bool>(true);
    });

    bool ret = PreviewHelper::instance()->isPreviewEnabled();

    EXPECT_TRUE(ret);
}

TEST(UT_PreviewHelper, showThumbnailInRemote)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::value, []{
        return QVariant::fromValue<bool>(true);
    });

    bool ret = PreviewHelper::instance()->showThumbnailInRemote();

    EXPECT_TRUE(ret);
}

TEST(UT_PreviewHelper, showThumbnailInMtp)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::value, []{
        return QVariant::fromValue<bool>(true);
    });

    bool ret = PreviewHelper::instance()->showThumbnailInMtp();

    EXPECT_TRUE(ret);
}

TEST(UT_PreviewHelper, bindConfig)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigSynchronizer::watchChange, [ &isOk ]{
        isOk = true;
        return true;
    });

    PreviewHelper::instance()->bindConfig();

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewHelper, saveRemoteToConf)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::setValue, [ &isOk ]{
        isOk = true;
    });

    PreviewHelper::instance()->saveRemoteToConf(QVariant());

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewHelper, syncRemoteToAppSet)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&Application::setGenericAttribute, [ &isOk ]{
        isOk = true;
    });

    PreviewHelper::instance()->syncRemoteToAppSet("UT", "UT_TEST", QVariant());

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewHelper, isRemoteConfEqual)
{
    bool isOk = PreviewHelper::instance()->isRemoteConfEqual(QVariant::fromValue<bool>(true), QVariant::fromValue<bool>(true));

    EXPECT_TRUE(isOk);
}
