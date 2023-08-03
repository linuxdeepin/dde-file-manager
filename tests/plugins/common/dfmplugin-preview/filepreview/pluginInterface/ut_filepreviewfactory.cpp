// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "pluginInterface/filepreviewfactory.h"
#include "pluginInterface/previewpluginloader.h"
#include "views/unknowfilepreview.h"

#include <gtest/gtest.h>

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

DPFILEPREVIEW_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_FilePreviewFactory, keys)
{
    QMultiMap<int, QString> map;
    map.insert(1, "UT_TEST");
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::keyMap, [ &map ]{
        return map;
    });

    QStringList result = FilePreviewFactory::keys();

    EXPECT_TRUE(result.first() == "UT_TEST");
}

TEST(UT_FilePreviewFactory, create_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>, []{
        return nullptr;
    });

    UnknowFilePreview *result = qobject_cast<UnknowFilePreview *>(FilePreviewFactory::create("UT_TEST"));

    EXPECT_FALSE(result);
}

TEST(UT_FilePreviewFactory, create_two)
{
    UnknowFilePreview view;

    stub_ext::StubExt stub;
    stub.set_lamda(dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>, [ &view ]{
        return &view;
    });

    UnknowFilePreview *result = qobject_cast<UnknowFilePreview *>(FilePreviewFactory::create("UT_TEST"));

    EXPECT_TRUE(result);
}

TEST(UT_FilePreviewFactory, isSuitedWithKey_one)
{
    bool isOk = FilePreviewFactory::isSuitedWithKey(nullptr, "UT_TEST");

    EXPECT_FALSE(isOk);
}

TEST(UT_FilePreviewFactory, isSuitedWithKey_two)
{
    UnknowFilePreview view;
    FilePreviewFactory::previewToLoaderIndex.insert(&view, 1);

    bool isOk = FilePreviewFactory::isSuitedWithKey(&view, "UT_TEST");

    EXPECT_FALSE(isOk);
}
