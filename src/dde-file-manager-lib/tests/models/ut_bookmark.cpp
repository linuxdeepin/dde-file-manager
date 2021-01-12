#include "models/bookmark.h"
#include "controllers/bookmarkmanager.h"
#include "dfmevent.h"

#include <gtest/gtest.h>

namespace {
class TestBookMark : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBookMark\n";

        bm = new BookMark("Test", DUrl("file:///usr/bin"));
    }

    void TearDown() override
    {
        std::cout << "end TestBookMark\n";
        delete bm;
    }

public:
    BookMark *bm;
};
} // namespace

TEST_F(TestBookMark, tstConstructorWithoutName)
{
    BookMark b(DUrl("/home"));
    EXPECT_TRUE(b.exists());
}

TEST_F(TestBookMark, tstBasicProperties)
{
    auto u = bm->sourceUrl();
    EXPECT_TRUE(u.path() == "/usr/bin");

    auto name = bm->fileDisplayName();
    EXPECT_TRUE(name == "Test");

    auto mpt = bm->getMountPoint();
    EXPECT_FALSE(!mpt.isEmpty());

    EXPECT_TRUE(bm->canRedirectionFileUrl());

    u = bm->redirectedFileUrl();
    EXPECT_FALSE(!u.isValid());

    u = bm->parentUrl();
    EXPECT_FALSE(!u.isValid());

    auto dt = bm->created();
    EXPECT_FALSE(dt.isValid());
    dt = bm->lastModified();
    EXPECT_FALSE(dt.isValid());
}

