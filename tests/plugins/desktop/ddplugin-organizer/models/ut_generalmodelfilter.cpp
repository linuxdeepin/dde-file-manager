// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/generalmodelfilter.h"
#include "models/filters/hiddenfilefilter.h"
#include "models/filters/innerdesktopappfilter.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

TEST(GeneralModelFilter, construct)
{
    GeneralModelFilter obj;
    ASSERT_EQ(obj.modelFilters.size(), 2);
    EXPECT_NE(dynamic_cast<HiddenFileFilter *>(obj.modelFilters.at(0)), nullptr);
    EXPECT_NE(dynamic_cast<InnerDesktopAppFilter *>(obj.modelFilters.at(1)), nullptr);
}

TEST(GeneralModelFilter, installFilter)
{
    GeneralModelFilter obj;
    obj.modelFilters.clear();

    auto cur = new ModelDataHandler();
    obj.installFilter(cur);
    ASSERT_EQ(obj.modelFilters.size(), 1);
    EXPECT_EQ(obj.modelFilters.at(0), cur);
    delete cur;
}

TEST(GeneralModelFilter, removeFilter)
{
    GeneralModelFilter obj;
    obj.modelFilters.clear();

    auto cur = new ModelDataHandler();
    obj.modelFilters << cur;

    obj.removeFilter(cur);
    EXPECT_EQ(obj.modelFilters.size(), 0);
    delete cur;
}

namespace testing {

class TestModelFilter : public ModelDataHandler
{
public:
    explicit TestModelFilter()
        : ModelDataHandler() {}
    bool acceptInsert(const QUrl &url) override
    {
        instert = true;
        return false;
    }
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override
    {
        reset = true;
        return {};
    }
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override
    {
        rename = true;
        return false;
    }
    bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {}) override
    {
        update = true;
        return false;
    }

public:
    bool instert = false;
    bool reset = false;
    bool rename = false;
    bool update = false;
};

}

class GeneralModelFilterTest : public testing::Test
{
public:
    virtual void SetUp() override
    {
        filter = new testing::TestModelFilter();
        obj.modelFilters.clear();
        obj.modelFilters << filter;
    }
    virtual void TearDown() override
    {
        delete filter;
    }
    GeneralModelFilter obj;
    testing::TestModelFilter *filter;
};

TEST_F(GeneralModelFilterTest, acceptInsert)
{
    QUrl cur("file://usr");
    EXPECT_FALSE(obj.acceptInsert(cur));
    EXPECT_TRUE(filter->instert);
    EXPECT_FALSE(filter->reset);
    EXPECT_FALSE(filter->rename);
    EXPECT_FALSE(filter->update);
}

TEST_F(GeneralModelFilterTest, acceptReset)
{
    QUrl cur("file://usr");
    EXPECT_TRUE(obj.acceptReset({ cur }).isEmpty());
    EXPECT_FALSE(filter->instert);
    EXPECT_TRUE(filter->reset);
    EXPECT_FALSE(filter->rename);
    EXPECT_FALSE(filter->update);
}

TEST_F(GeneralModelFilterTest, acceptRename)
{
    QUrl cur("file://usr");
    EXPECT_FALSE(obj.acceptRename(QUrl(), cur));
    EXPECT_FALSE(filter->instert);
    EXPECT_FALSE(filter->reset);
    EXPECT_TRUE(filter->rename);
    EXPECT_FALSE(filter->update);
}

TEST_F(GeneralModelFilterTest, acceptUpdate)
{
    QUrl cur("file://usr");
    EXPECT_FALSE(obj.acceptUpdate(cur));
    EXPECT_FALSE(filter->instert);
    EXPECT_FALSE(filter->reset);
    EXPECT_FALSE(filter->rename);
    EXPECT_TRUE(filter->update);
}
