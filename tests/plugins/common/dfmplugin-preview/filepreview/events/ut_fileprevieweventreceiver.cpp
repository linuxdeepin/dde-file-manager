// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/fileprevieweventreceiver.h"
#include "utils/previewhelper.h"
#include "utils/previewdialogmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPFILEPREVIEW_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(UT_FilePreviewEventReceiver, connectService)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef bool(EventChannelManager::*FuncType)(const QString &, const QString &, FilePreviewEventReceiver *, void(FilePreviewEventReceiver::*)(quint64, const QList<QUrl> &, QList<QUrl>));
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::connect), [ &isOk ]{
        isOk = true;
        return true;
    });

    FilePreviewEventReceiver::instance()->connectService();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewEventReceiver, showFilePreview)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewHelper::isPreviewEnabled, []{
        return true;
    });
    stub.set_lamda(&PreviewDialogManager::showPreviewDialog, [ &isOk ]{
        isOk = true;
    });

    FilePreviewEventReceiver::instance()->showFilePreview(0, QList<QUrl>() << QUrl("file:///UT_TEST"), QList<QUrl>() << QUrl("file:///UT_TEST1"));

    EXPECT_TRUE(isOk);
}

