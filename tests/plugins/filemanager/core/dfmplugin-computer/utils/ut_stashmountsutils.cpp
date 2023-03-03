// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-computer/utils/stashmountsutils.h"
#include "plugins/filemanager/core/dfmplugin-computer/utils/computerutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"

#include <QSet>
#include <QMap>
#include <QTextStream>

#include <gtest/gtest.h>

DPCOMPUTER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_StashMountsUtils : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_StashMountsUtils, IsStashMountsEnable)
{
    stub.set_lamda(Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return QVariant(true); });
    EXPECT_TRUE(StashMountsUtils::isStashMountsEnabled());
}

TEST_F(UT_StashMountsUtils, StashedConfigPath)
{
    EXPECT_TRUE(StashMountsUtils::stashedConfigPath().endsWith("/deepin/dde-file-manager.json"));
}

TEST_F(UT_StashMountsUtils, StashedMounts)
{
    stub.set_lamda(&Settings::keys, [] { __DBG_STUB_INVOKE__ return QSet<QString> { "ftp://127.0.0.1/util-dfm/" }; });
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [] { __DBG_STUB_INVOKE__ return QVariant(QVariantMap {
                                                                      { "protocol", "smb" },
                                                                      { "host", "127.0.0.1" },
                                                                      { "share", "util-dfm" },
                                                                      { "name", "util-dfm" } }); });
    EXPECT_TRUE(StashMountsUtils::stashedMounts().count() > 0);
    EXPECT_TRUE(StashMountsUtils::stashedMounts().firstKey() == "smb://127.0.0.1/util-dfm/");
}

TEST_F(UT_StashMountsUtils, DisplayName)
{
    QStringList rets { "", "smb://1.2.3.4" };
    stub.set_lamda(ComputerUtils::getProtocolDevIdByStashedUrl, [&rets] { __DBG_STUB_INVOKE__ return rets.takeFirst(); });
    stub.set_lamda(StashMountsUtils::stashedMounts, [] { __DBG_STUB_INVOKE__ return QMap<QString, QString> { { "smb://1.2.3.4", "hello" } }; });
    EXPECT_TRUE(StashMountsUtils::displayName(QUrl("")) == QObject::tr("Unknown"));
    EXPECT_TRUE(StashMountsUtils::displayName(QUrl("hello")) == "hello");
}

TEST_F(UT_StashMountsUtils, RemoveStashedMount) {}

TEST_F(UT_StashMountsUtils, StashMount)
{
    bool enable = false;
    stub.set_lamda(StashMountsUtils::isStashMountsEnabled, [&enable] { __DBG_STUB_INVOKE__ return enable; });
    stub.set_lamda(ComputerUtils::getProtocolDevIdByUrl, [] { __DBG_STUB_INVOKE__ return ""; });
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(StashMountsUtils::stashMount(QUrl(), "hello"));
    enable = true;
    EXPECT_NO_FATAL_FAILURE(StashMountsUtils::stashMount(QUrl(""), "hshhshs"));
}

TEST_F(UT_StashMountsUtils, ClearStashedMounts)
{
    stub.set_lamda(&Settings::removeGroup, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(StashMountsUtils::clearStashedMounts());
}

TEST_F(UT_StashMountsUtils, IsStashedDevExists)
{
    stub.set_lamda(ComputerUtils::getProtocolDevIdByStashedUrl, [] { __DBG_STUB_INVOKE__ return ""; });
    stub.set_lamda(StashMountsUtils::stashedMounts, [] { __DBG_STUB_INVOKE__ return QMap<QString, QString> { { "smb://1.2.3.4", "hello" } }; });
    EXPECT_NO_FATAL_FAILURE(StashMountsUtils::isStashedDevExist(QUrl()));
}

TEST_F(UT_StashMountsUtils, StashMountedMounts)
{
    stub.set_lamda(&DeviceProxyManager::getAllProtocolIds, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4", "ftp://1.2.3.4" }; });
    stub.set_lamda(&EntryFileInfo::displayName, [] { __DBG_STUB_INVOKE__ return "hello"; });
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(StashMountsUtils::stashMountedMounts());
}

TEST_F(UT_StashMountsUtils, CfgDocument)
{
    stub.set_lamda(&StashMountsUtils::stashedConfigPath, [] { __DBG_STUB_INVOKE__ return "/tmp/test_cfg.json"; });
    EXPECT_TRUE(StashMountsUtils::cfgDocument().isEmpty());

    QFile f(StashMountsUtils::stashedConfigPath());
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream s(&f);
    s << R"({"hello": "world"})";
    f.close();

    EXPECT_FALSE(StashMountsUtils::cfgDocument().isEmpty());
    ::remove(StashMountsUtils::stashedConfigPath().toStdString().c_str());
}
