#include "diskpluginitem.h"

#include <QWidget>
#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

namespace  {
    class TestDiskPluginItem : public testing::Test {
    public:

        void SetUp() override
        {
            mDiskPluginItem.reset( new DiskPluginItem());
        }
        void TearDown() override
        {
        }

    public:
        std::shared_ptr<DiskPluginItem> mDiskPluginItem;
    };
}

TEST_F(TestDiskPluginItem, can_set_display_mode)
{
    mDiskPluginItem->show();

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    mDiskPluginItem->setDockDisplayMode(Dock::Fashion);

    mDiskPluginItem->hide();
}

TEST_F(TestDiskPluginItem, can_be_updated_from_signals)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    mDiskPluginItem->setDockDisplayMode(Dock::Fashion);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
}
