// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/common/dfmplugin-burn/utils/burnjob.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QSet>
#include <QWidget>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_BurnJob : public testing::Test
{
public:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnJob, currentDeviceInfo)
{
    EraseJob job { "/dev/sr0", nullptr };
    job.curDeviceInfo["test"] = "test";
    EXPECT_EQ(job.curDeviceInfo, job.currentDeviceInfo());
}

TEST_F(UT_BurnJob, property)
{
    EraseJob job { "/dev/sr0", nullptr };
    job.setProperty(AbstractBurnJob::PropertyType::kSpeeds, QVariant::fromValue(10));
    EXPECT_EQ(job.property(AbstractBurnJob::PropertyType::kSpeeds), QVariant::fromValue(10));
}

TEST_F(UT_BurnJob, readyToWork_emptyInfo)
{
    EraseJob job { "/dev/sr0", nullptr };
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        return QVariantMap {};
    });
    EXPECT_FALSE(job.readyToWork());
}

TEST_F(UT_BurnJob, readyToWork_blank)
{
    EraseJob job { "/dev/sr0", nullptr };

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = true;
        return map;
    });
    EXPECT_TRUE(job.readyToWork());

    // is dvd+rw/-rw
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = false;
        map[DeviceProperty::kSizeTotal] = 1024;
        map[DeviceProperty::kSizeFree] = 1024;
        return map;
    });
    EXPECT_TRUE(job.readyToWork());
}

TEST_F(UT_BurnJob, readyToWork_unmount)
{
    EraseJob job { "/dev/sr0", nullptr };

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = false;
        map[DeviceProperty::kMountPoint] = "/media/test/test";
        return map;
    });

    stub.set_lamda(ADDR(DeviceManager, unmountBlockDev), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(job.readyToWork());

    // umount failed
    stub.set_lamda(ADDR(DeviceManager, unmountBlockDev), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_FALSE(job.readyToWork());
}

TEST_F(UT_BurnJob, readyToWork)
{
    EraseJob job { "/dev/sr0", nullptr };

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = true;
        map[DeviceProperty::kMountPoint] = QString();
        return map;
    });

    EXPECT_TRUE(job.readyToWork());
}

TEST_F(UT_BurnJob, onJobUpdated)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };

    stub.set_lamda(ADDR(Settings, groups), [] {
        __DBG_STUB_INVOKE__
        return QSet<QString> {};
    });
    stub.set_lamda(ADDR(BurnHelper, updateBurningStateToPersistence), [] {
        __DBG_STUB_INVOKE__
    });
    // failed
    job.onJobUpdated(DFMBURN::JobStatus::kFailed, 30, {}, {});
    EXPECT_EQ(job.lastProgress, 30);
}

TEST_F(UT_BurnJob, onJobUpdated_Success)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };

    stub.set_lamda(ADDR(Settings, groups), [] {
        __DBG_STUB_INVOKE__
        return QSet<QString> {};
    });
    stub.set_lamda(ADDR(BurnHelper, updateBurningStateToPersistence), [] {
        __DBG_STUB_INVOKE__
    });

    job.onJobUpdated(DFMBURN::JobStatus::kFinished, 100, "4x", {});
    EXPECT_EQ(job.lastProgress, 100);
    EXPECT_EQ(job.lastStatus, DFMBURN::JobStatus::kFinished);
}

TEST_F(UT_BurnJob, onJobUpdated_Running)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };

    stub.set_lamda(ADDR(Settings, groups), [] {
        __DBG_STUB_INVOKE__
        return QSet<QString> {};
    });
    stub.set_lamda(ADDR(BurnHelper, updateBurningStateToPersistence), [] {
        __DBG_STUB_INVOKE__
    });

    job.onJobUpdated(DFMBURN::JobStatus::kRunning, 50, "2x", { "Processing files" });
    EXPECT_EQ(job.lastProgress, 50);
    EXPECT_EQ(job.lastStatus, DFMBURN::JobStatus::kRunning);
}

TEST_F(UT_BurnJob, addTask)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };

    // Test that addTask doesn't crash
    job.addTask();
}

TEST_F(UT_BurnJob, setProperty_GetProperty)
{
    EraseJob job { "/dev/sr0", nullptr };

    // Test string property
    job.setProperty(AbstractBurnJob::PropertyType::kVolumeName, QString("TestVolume"));
    EXPECT_EQ(job.property(AbstractBurnJob::PropertyType::kVolumeName).toString(), "TestVolume");

    // Test int property
    job.setProperty(AbstractBurnJob::PropertyType::kSpeeds, 4);
    EXPECT_EQ(job.property(AbstractBurnJob::PropertyType::kSpeeds).toInt(), 4);

    // Test URL property
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.iso");
    job.setProperty(AbstractBurnJob::PropertyType::kImageUrl, testUrl);
    EXPECT_EQ(job.property(AbstractBurnJob::PropertyType::kImageUrl).toUrl(), testUrl);
}

TEST_F(UT_BurnJob, EraseJob_Constructor)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    EraseJob job { "/dev/sr0", jobHandler };

    EXPECT_EQ(job.curDev, "/dev/sr0");
    EXPECT_EQ(job.firstJobType, AbstractBurnJob::JobType::kOpticalBlank);
}

TEST_F(UT_BurnJob, BurnISOFilesJob_Constructor)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    BurnISOFilesJob job { "/dev/sr1", jobHandler };

    EXPECT_EQ(job.curDev, "/dev/sr1");
    EXPECT_EQ(job.firstJobType, AbstractBurnJob::JobType::kOpticalBurn);
}

TEST_F(UT_BurnJob, BurnISOImageJob_Constructor)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    BurnISOImageJob job { "/dev/sr2", jobHandler };

    EXPECT_EQ(job.curDev, "/dev/sr2");
    EXPECT_EQ(job.firstJobType, AbstractBurnJob::JobType::kOpticalImageBurn);
}

TEST_F(UT_BurnJob, BurnUDFFilesJob_Constructor)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    BurnUDFFilesJob job { "/dev/sr3", jobHandler };

    EXPECT_EQ(job.curDev, "/dev/sr3");
    EXPECT_EQ(job.firstJobType, AbstractBurnJob::JobType::kOpticalBurn);
}

TEST_F(UT_BurnJob, DumpISOImageJob_Constructor)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DumpISOImageJob job { "/dev/sr4", jobHandler };

    EXPECT_EQ(job.curDev, "/dev/sr4");
    EXPECT_EQ(job.firstJobType, AbstractBurnJob::JobType::kOpticalImageDump);
}

TEST_F(UT_BurnJob, readyToWork_NotBlankButCanErase)
{
    EraseJob job { "/dev/sr0", nullptr };

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = false;
        map[DeviceProperty::kSizeTotal] = 1024;
        map[DeviceProperty::kSizeFree] = 0;   // Different from total, so not erasable
        return map;
    });

    EXPECT_FALSE(job.readyToWork());
}

TEST_F(UT_BurnJob, mediaChangDected)
{
    EraseJob job { "/dev/sr0", nullptr };

    // Set initial device info
    job.curDeviceInfo[DeviceProperty::kDevice] = "/dev/sr0";

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kDevice] = "/dev/sr1";   // Different device
        return map;
    });

    EXPECT_TRUE(job.mediaChangDected());
}

TEST_F(UT_BurnJob, mediaChangDected_NoChange)
{
    EraseJob job { "/dev/sr0", nullptr };

    // Set initial device info
    job.curDeviceInfo[DeviceProperty::kDevice] = "/dev/sr0";

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kDevice] = "/dev/sr0";   // Same device
        return map;
    });

    EXPECT_FALSE(job.mediaChangDected());
}

TEST_F(UT_BurnJob, comfort)
{
    stub.set_lamda(ADDR(AbstractBurnJob, onJobUpdated), []() {
        __DBG_STUB_INVOKE__
    });

    EraseJob job { "/dev/sr0", nullptr };

    // Test that comfort method doesn't crash
    job.comfort();
}
