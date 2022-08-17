/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include "plugins/filemanager/dfmplugin-optical/utils/opticalhelper.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <gtest/gtest.h>

class UT_OpticalHelper : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

//TEST_F(UT_OpticalHelper, Scheme) {}
//TEST_F(UT_OpticalHelper, Icon) {}
//TEST_F(UT_OpticalHelper, IconString) {}
//TEST_F(UT_OpticalHelper, LocalStagingRoot) {}
//TEST_F(UT_OpticalHelper, LocalStagingFile) {}
//TEST_F(UT_OpticalHelper, LocalDiscFile) {}
//TEST_F(UT_OpticalHelper, BurnDestDevice) {}
//TEST_F(UT_OpticalHelper, BurnFilePath) {}
//TEST_F(UT_OpticalHelper, BurnIsOnDisc) {}
//TEST_F(UT_OpticalHelper, BurnIsonStaging) {}
//TEST_F(UT_OpticalHelper, TransToBurnFile) {}
//TEST_F(UT_OpticalHelper, TransToLocalFile) {}
//TEST_F(UT_OpticalHelper, IsSupportedUDFVersion) {}
//TEST_F(UT_OpticalHelper, IsSupportedUDFMedium) {}
//TEST_F(UT_OpticalHelper, CreateStagingFolder) {}
//TEST_F(UT_OpticalHelper, IsDupFileNameInPath) {}

TEST_F(UT_OpticalHelper, IsBurnEnabled)
{
    stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(OpticalHelper::isBurnEnabled());
}

//TEST_F(UT_OpticalHelper, UrlsToLocal) {}
//TEST_F(UT_OpticalHelper, IsTransparent) {}
