// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_BurnJob : public testing::Test
{
public:
    virtual void SetUp() override
    {
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
