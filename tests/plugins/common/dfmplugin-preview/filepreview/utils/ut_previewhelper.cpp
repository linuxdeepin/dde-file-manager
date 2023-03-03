// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"

#include "plugins/common/dfmplugin-preview/filepreview/utils/previewhelper.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <gtest/gtest.h>
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;
class UT_PreviewHelper : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};
TEST_F(UT_PreviewHelper, IsPreviewEnabled)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return true; });
    const auto &&ret1 = DConfigManager::instance()->value("org.deepin.dde.file-manager.preview", "previewEnable");
    EXPECT_TRUE(ret1.isValid() ? ret1.toBool() : true);
    stub.clear();
}
