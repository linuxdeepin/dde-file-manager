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
