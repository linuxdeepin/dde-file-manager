// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "filepreview.h"
#include "events/fileprevieweventreceiver.h"
#include "utils/previewhelper.h"

#include <gtest/gtest.h>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

DPFILEPREVIEW_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(UT_FilePreview, initialize)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewEventReceiver::connectService, [ &isOk ]{
        isOk = true;
    });

    FilePreview plugin;
    plugin.initialize();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreview, start)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::addConfig, []{
        return false;
    });
    stub.set_lamda(&PreviewHelper::bindConfig, []{});

    FilePreview plugin;
    bool isOk = plugin.start();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreview, onConfigChanged)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isOk ]{
        isOk = true;
        return true;
    });

    FilePreview plugin;
    plugin.onConfigChanged("UT_TEST", "UT");
    plugin.onConfigChanged(ConfigInfos::kConfName, ConfigInfos::kMtpThumbnailKey);

    EXPECT_TRUE(isOk);
}
