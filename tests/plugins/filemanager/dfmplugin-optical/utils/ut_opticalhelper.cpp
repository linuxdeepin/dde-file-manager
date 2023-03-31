// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-optical/utils/opticalhelper.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

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
