#include "diskcontrolwidget.h"

#include <QWidget>
#include <gtest/gtest.h>

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmsettings.h>
#include <dgiosettings.h>
#include <DDesktopServices>


namespace  {
    class TestDiskControlWidget : public testing::Test {
    public:

        void SetUp() override
        {
            mCtrlWidget.reset( new DiskControlWidget());
            mDiskManager = mCtrlWidget->startMonitor();
        }
        void TearDown() override
        {
        }

    public:
        DDiskManager* mDiskManager;
        std::shared_ptr<DiskControlWidget> mCtrlWidget;
    };
}

static const QString BLK_DEVICE_MOUNT_POINT = "/midea/max";

TEST_F(TestDiskControlWidget, can_send_notifymsg)
{
    mCtrlWidget->NotifyMsg("msg details");
    mCtrlWidget->NotifyMsg("title", "msg informations");
}

TEST_F(TestDiskControlWidget, can_monitor_diskDeviceRemoved)
{
    emit mDiskManager->diskDeviceRemoved(BLK_DEVICE_MOUNT_POINT);
}
