// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"

#include "plugins/common/dfmplugin-preview/filepreview/utils/previewdialogmanager.h"
#include "plugins/common/dfmplugin-preview/filepreview/utils/previewhelper.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;
class UT_FilePreviewEventReceiver : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};
TEST_F(UT_FilePreviewEventReceiver, ShowFilePreview)
{
    const QList<QUrl>inputs1 = { QUrl::fromLocalFile("/") };
    const QList<QUrl>inputs2 = { QUrl::fromLocalFile("/Hello") };
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewHelper::isPreviewEnabled, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&PreviewDialogManager::showPreviewDialog, [](PreviewDialogManager *,const quint64, const QList<QUrl> &, const QList<QUrl>){});
    EXPECT_NO_FATAL_FAILURE(PreviewDialogManager::instance()->showPreviewDialog(0,inputs1,inputs2));
}
