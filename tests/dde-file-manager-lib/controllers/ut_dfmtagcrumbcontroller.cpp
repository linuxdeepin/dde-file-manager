// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmtagcrumbcontroller.h"

using DFM_NAMESPACE::DFMTagCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class TestDFMTagCrumbController : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMTagCrumbController m_pController;
    };
}

TEST_F(TestDFMTagCrumbController, supportedUrl)
{
    DUrl url;
    url.setScheme(TAG_SCHEME);
    EXPECT_TRUE(m_pController.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(m_pController.supportedUrl(url));
}

TEST_F(TestDFMTagCrumbController, seprateUrl)
{
    DUrl url;
    url.setScheme(TAG_SCHEME);
    QList<CrumbData> list = m_pController.seprateUrl(url);
    EXPECT_TRUE(!list.empty());
}
