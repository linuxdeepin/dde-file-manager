#include "plugins/pluginmanager.h"

#include <gtest/gtest.h>

namespace  {
    class TestPluginManager : public testing::Test
    {

    };
}

TEST_F(TestPluginManager, testInit)
{
    PluginManager::instance();
}

TEST_F(TestPluginManager, testPluginDir)
{
    QString result = PluginManager::instance()->PluginDir();
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestPluginManager, testLoadPlugin)
{
    PluginManager::instance()->loadPlugin();
}

TEST_F(TestPluginManager, testGetViewInterfaceByScheme)
{
    QString scheme = "file";
    PluginManager::instance()->getViewInterfaceByScheme(scheme);
}
