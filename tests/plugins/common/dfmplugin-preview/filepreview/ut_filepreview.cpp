// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "addr_any.h"

#include "plugins/common/dfmplugin-preview/filepreview/filepreview.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;
class UT_FilePreview : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};
TEST_F(UT_FilePreview, Start)
{
    QString err;
    stub_ext::StubExt stub;
    stub.set_lamda(&DConfigManager::addConfig, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.preview", &err));
    stub.clear();
}
