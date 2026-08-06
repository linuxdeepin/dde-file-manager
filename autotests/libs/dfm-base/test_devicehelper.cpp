// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicehelper.cpp
 * @brief Unit tests for DeviceHelper (base/device/private/devicehelper.cpp) —
 *        the dependency-light subset: castFromDFMMountProperty (pure lookup),
 *        isMountableBlockDev/isEjectableBlockDev (QVariantMap overloads, pure
 *        logic), clearOpticalInfo (empty-tag early return), makeFakeProtocolInfo
 *        (private, exercised via -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QString>
#include <dfm-mount/base/dmount_global.h>

#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/dbusservice/global_server_defines.h>

using namespace dfmbase;

TEST(DeviceHelperTest, CastFromDFMMountPropertyKnownProperty)
{
    using namespace dfmmount;
    QString result = DeviceHelper::castFromDFMMountProperty(Property::kBlockSize);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceHelperTest, CastFromDFMMountPropertyMultipleKnown)
{
    using namespace dfmmount;
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDUUID).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDType).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMedia).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveOptical).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveEjectable).isEmpty());
}

TEST(DeviceHelperTest, CastFromDFMMountPropertyUnknownReturnsEmpty)
{
    using namespace dfmmount;
    // A property not in the mapper returns ""
    EXPECT_EQ(DeviceHelper::castFromDFMMountProperty(static_cast<Property>(99999)), QString());
}

namespace DP = GlobalServerDefines::DeviceProperty;

TEST(DeviceHelperTest, IsMountableBlockDevEmptyIdReturnsFalse)
{
    QVariantMap infos;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevHintIgnoreReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHintIgnore] = true;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevAlreadyMountedReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kMountPoint] = "/mnt/data";
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevNoFileSystemReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = false;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsMountableBlockDevEncryptedReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = true;
    infos[DP::kIsEncrypted] = true;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsMountableBlockDevValidReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = true;
    infos[DP::kIsEncrypted] = false;
    QString why;
    EXPECT_TRUE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevRemovableReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kRemovable] = true;
    QString why;
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevOpticalEjectableReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kOptical] = true;
    infos[DP::kEjectable] = true;
    QString why;
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevNonEjectableReturnsFalse)
{
    QVariantMap infos;
    QString why;
    EXPECT_FALSE(DeviceHelper::isEjectableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, ClearOpticalInfoEmptyTagIsNoOp)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceHelper::clearOpticalInfo(QString()); });
}

TEST(DeviceHelperTest, MakeFakeProtocolInfoBuildsBasicMap)
{
    // Private method, reached via -fno-access-control.
    QString id = "smb://10.0.0.1/share";
    QVariantMap info = DeviceHelper::makeFakeProtocolInfo(id);
    EXPECT_FALSE(info.isEmpty());
    EXPECT_EQ(info.value("fake").toBool(), true);
}
