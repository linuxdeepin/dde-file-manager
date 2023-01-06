// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/dfmcomputercrumbcontroller.h"

DFM_USE_NAMESPACE



namespace  {

class TestDFMComputerCrumbController : public testing::Test
{
public:
    DFMComputerCrumbController dfmComputerCrubCtl;
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestDFMComputerCrumbController, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_FALSE(dfmComputerCrubCtl.supportedUrl(url));
}

TEST_F(TestDFMComputerCrumbController, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmComputerCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
