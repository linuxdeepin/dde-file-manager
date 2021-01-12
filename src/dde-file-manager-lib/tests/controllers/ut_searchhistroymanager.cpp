#include <gtest/gtest.h>

#include "controllers/searchhistroymanager.h"

namespace  {
class TestSearchHistroyManager : public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestSearchHistroyManager" << std::endl;
        manager = new SearchHistroyManager();
    }

    virtual void TearDown() override
    {
        std::cout << "end TestSearchHistroyManager" << std::endl;
        if (manager) {
            delete manager;
            manager = nullptr;
        }
    }

public:
    SearchHistroyManager *manager;
};
}

TEST_F(TestSearchHistroyManager, toStringList)
{
    manager->writeIntoSearchHistory("keyworld");
    manager->writeIntoSearchHistory("");
    auto res = manager->toStringList();
    EXPECT_TRUE(!res.isEmpty());
}

TEST_F(TestSearchHistroyManager, removeSearchHistory)
{
    auto res = manager->removeSearchHistory("");
    EXPECT_FALSE(res);

    res = manager->removeSearchHistory("not exist");
    EXPECT_FALSE(res);

    res = manager->removeSearchHistory("keyworld");
    EXPECT_TRUE(res);
}

TEST_F(TestSearchHistroyManager, clearHistory)
{
    manager->clearHistory();
    auto res = manager->toStringList();
    EXPECT_TRUE(res.isEmpty());
}
