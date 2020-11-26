#include <gtest/gtest.h>

#define private public
#define protected public

#include "plugins/pluginmanager.h"
#include "interfaces/dfmglobal.h"

namespace  {
    class PluginManagerTest : public testing::Test
    {
    public:
        void SetUp() override {
            p_manager = PluginManager::instance();
        }
        void TearDown() override {
            p_manager = nullptr;
        }

        PluginManager   *p_manager = nullptr;
    };
}

TEST_F(PluginManagerTest, get_plugin_dir)
{
    ASSERT_NE(p_manager, nullptr);

    QString result = p_manager->PluginDir();
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(PluginManagerTest, load_plugin_success)
{
    ASSERT_NE(p_manager, nullptr);

    DFMGlobal::PluginLibraryPaths.append(QStringList()<<QString("/")<<QString("/usr/lib"));
    p_manager->loadPlugin();
}

TEST_F(PluginManagerTest, get_expandInfo_interfaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<PropertyDialogExpandInfoInterface *> result = p_manager->getExpandInfoInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->expandInfoInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<ViewInterface *> result = p_manager->getViewInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->viewInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfacesMap)
{
    ASSERT_NE(p_manager, nullptr);

    QMap<QString, ViewInterface *> result = p_manager->getViewInterfacesMap();
    EXPECT_EQ(result.count(), p_manager->d_func()->viewInterfacesMap.count());
}

TEST_F(PluginManagerTest, get_preview_inteefaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<PreviewInterface *> result = p_manager->getPreviewInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->previewInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfaceByScheme)
{
    ASSERT_NE(p_manager, nullptr);

    QString scheme = "file";
    ViewInterface * result = p_manager->getViewInterfaceByScheme(scheme);
    if(p_manager->d_func()->viewInterfacesMap.contains(scheme)) {
        EXPECT_EQ(result, p_manager->d_func()->viewInterfacesMap.value(scheme));
    } else {
        EXPECT_EQ(result, nullptr);
    }

    QMap<QString, ViewInterface *> tmpMap = p_manager->getViewInterfacesMap();
    if (!tmpMap.isEmpty()) {
        scheme = tmpMap.firstKey();
        result = p_manager->getViewInterfaceByScheme(scheme);
        ASSERT_NE(result, nullptr);
    }
}
