#include "ut_mock_stub_disk_gio.h"
#include "diskcontrolitem.h"
#include "ut_mock_dattacheddeviceinterface.h"
#include "ut_mock_stub_diskdevice.h"

#include "stub.h"

#include <QWidget>
#include <gtest/gtest.h>
#include <QtTest/QtTest>


namespace  {
    class TestDiskControlItem : public testing::Test {
    public:

        void SetUp() override
        {
            mockInterface = new MockDAttachedDeviceInterface();
            mDiskContrlItem.reset( new DiskControlItem(mockInterface));

            testing::Mock::AllowLeak(mockInterface);
        }
        void TearDown() override
        {
        }

    public:

        MockDAttachedDeviceInterface* mockInterface;
        std::shared_ptr<DiskControlItem>mDiskContrlItem;
    };
}

TEST_F(TestDiskControlItem, can_format_disk_size)
{
    EXPECT_EQ("1 KB", mDiskContrlItem->formatDiskSize(1024) );
}

TEST_F(TestDiskControlItem, can_set_tag_name)
{
    QString tagName = "test tag";
    mDiskContrlItem->setTagName(tagName);
    EXPECT_EQ(tagName, mDiskContrlItem->tagName() );
}

TEST_F(TestDiskControlItem, can_be_showed)
{
    //EXPECT_CALL(*mockInterface,displayName).WillOnce(testing::Return("displayName"));
    EXPECT_CALL(*mockInterface,deviceUsageValid).WillOnce(testing::Return(false));
    ON_CALL(*mockInterface,iconName).WillByDefault(testing::Return("iconName"));

    ON_CALL(*mockInterface,deviceUsage).WillByDefault(testing::Return(qMakePair(1024, 1024 * 1024)));

    mDiskContrlItem->show();
    mDiskContrlItem->hide();
}

TEST_F(TestDiskControlItem, showed_with_deviceUsageValid)
{
    //EXPECT_CALL(*mockInterface,displayName).WillOnce(testing::Return("displayName"));
    EXPECT_CALL(*mockInterface,iconName).WillOnce(testing::Return("iconName"));

    EXPECT_CALL(*mockInterface,deviceUsageValid).WillOnce(testing::Return(true));

    ON_CALL(*mockInterface,deviceUsage).WillByDefault(testing::Return(qMakePair(1024, 1024 * 1024)));

    mDiskContrlItem->show();
    mDiskContrlItem->hide();
}

TEST_F(TestDiskControlItem, showed_with_deviceUsageValid_and_media_optical)
{
    //EXPECT_CALL(*mockInterface,displayName).WillOnce(testing::Return("displayName"));
    EXPECT_CALL(*mockInterface,iconName).WillOnce(testing::Return("media-optical"));

    EXPECT_CALL(*mockInterface,deviceUsageValid).WillOnce(testing::Return(true));

    ON_CALL(*mockInterface,deviceUsage).WillByDefault(testing::Return(qMakePair(1024, 1024 * 1024)));

    mDiskContrlItem->show();
    mDiskContrlItem->hide();
}

TEST_F(TestDiskControlItem, showed_with_tagname_and_media_optical)
{
    //EXPECT_CALL(*mockInterface,displayName).WillOnce(testing::Return("displayName"));
    EXPECT_CALL(*mockInterface,iconName).WillOnce(testing::Return("media-optical"));

    EXPECT_CALL(*mockInterface,deviceUsageValid).WillOnce(testing::Return(true));

    ON_CALL(*mockInterface,deviceUsage).WillByDefault(testing::Return(qMakePair(1024, 1024 * 1024)));

    QString tagName = "sr0";
    mDiskContrlItem->setTagName(tagName);
    EXPECT_EQ(tagName, mDiskContrlItem->tagName() );

    mDiskContrlItem->show();
    mDiskContrlItem->hide();
}

TEST_F(TestDiskControlItem, kick_mouse_open_normal_path)
{
    EXPECT_CALL(*mockInterface,accessPointUrl).WillOnce(testing::Return(QUrl(getAppRunPath())));

    QTest::mouseRelease(mDiskContrlItem.get(), Qt::MouseButton::LeftButton);
}

TEST_F(TestDiskControlItem, kick_mouse_open_optical_path)
{
    EXPECT_CALL(*mockInterface,accessPointUrl).WillOnce(testing::Return(QUrl("burn:/dev/sr0/disc_files/")));

    QTest::mouseRelease(mDiskContrlItem.get(), Qt::MouseButton::LeftButton);
}

TEST_F(TestDiskControlItem, kick_mouse_open_optical_path_filemanager_notready)
{
    Stub stub;
    stub.set(ADDR(QStandardPaths, findExecutable), findExecutable_QStandardPaths_return_empty_stub);

    EXPECT_CALL(*mockInterface,accessPointUrl).WillRepeatedly(testing::Return(QUrl("burn:/dev/sr0/disc_files/")));
    EXPECT_CALL(*mockInterface,mountpointUrl).WillRepeatedly(testing::Return(QUrl(getAppRunPath())));


    QTest::mouseRelease(mDiskContrlItem.get(), Qt::MouseButton::LeftButton);
}
