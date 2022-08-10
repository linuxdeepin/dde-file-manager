/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     likai<likai@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
