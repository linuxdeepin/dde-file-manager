// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceproxymanagerprivate.cpp
 * @brief Unit tests for DeviceProxyManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/deviceproxymanager.h"

#include <QTest>

using namespace src;

class DeviceProxyManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceProxyManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceProxyManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceProxyManagerPrivateTest, matchMounts)
{
    // Test method: bool matchMounts((const QString &filePath,
                                            const QMap<QString, QStringList> &mounts,
                                            const std::function<bool(const QString &)> &devFilter))
    QString _arg0{};
    QMap<QString, QStringList> _arg1{};
    std::function<bool( QString )> _arg2{};
    auto result = obj->matchMounts(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(DeviceProxyManagerPrivateTest, removeMounts)
{
    // Test method: void removeMounts((const QString &id, const QString &mpt))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removeMounts(_arg0, _arg1));
}
