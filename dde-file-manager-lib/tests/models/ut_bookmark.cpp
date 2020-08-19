#include "models/bookmark.h"

#include <gtest/gtest.h>

namespace {
class TestBookMark : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBookMark";

        bm = new BookMark(DUrl("bookmark:///test"));
    }

    void TearDown() override
    {
        std::cout << "end TestBookMark";
    }

public:
    BookMark *bm;
};
} // namespace

TEST_F(TestBookMark, sourceUrl)
{
    EXPECT_STREQ("/test", bm->sourceUrl().path().toStdString().c_str());
}

TEST_F(TestBookMark, bookmarkName)
{
    EXPECT_STREQ("", bm->getName().toStdString().c_str());
}

TEST_F(TestBookMark, bookmarkIsExist)
{
    EXPECT_TRUE(bm->exists());
}

TEST_F(TestBookMark, fileDisplayName)
{
    EXPECT_STREQ("", bm->fileDisplayName().toStdString().c_str());
}

TEST_F(TestBookMark, canRedirectUrl)
{
    EXPECT_TRUE(bm->canRedirectionFileUrl());
}

TEST_F(TestBookMark, redirectUrl)
{
    EXPECT_STREQ("/test", bm->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestBookMark, parentUrl)
{
    EXPECT_STREQ("/", bm->parentUrl().path().toStdString().c_str());
}
