// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/dfmbookmarkcrumbcontroller.h"

DFM_USE_NAMESPACE



namespace  {

class TestDfmbookmarkcrumbcontroller : public testing::Test
{
public:
    DFMBookmarkCrumbController dfmBookmarkCrubCtl;
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestDfmbookmarkcrumbcontroller, tst_supportedUrl)
{
    DUrl url("file:///home");
    EXPECT_FALSE(dfmBookmarkCrubCtl.supportedUrl(url));
}

TEST_F(TestDfmbookmarkcrumbcontroller, tst_seprateUrl)
{
    DUrl url("file:///home");
    QList<CrumbData> crumbDataList = dfmBookmarkCrubCtl.seprateUrl(url);
    EXPECT_TRUE(!crumbDataList.isEmpty());
}
