// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "utils/smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/configs/settingbackend.cpp>

#include <QUrl>
#include <QIcon>
#include <QMutex>
#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace dfmplugin_smbbrowser;

class UT_SmbBrowserUtils : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SmbBrowserUtils, NetworkScheme)
{
    EXPECT_TRUE(smb_browser_utils::networkScheme() == "network");
}

TEST_F(UT_SmbBrowserUtils, NetNeighborRootUrl)
{
    QUrl u;
    u.setScheme("network");
    u.setPath("/");
    u.setHost("");
    auto rootUrl = smb_browser_utils::netNeighborRootUrl();
    EXPECT_EQ(rootUrl, u);
    EXPECT_EQ(rootUrl.path(), "/");
    EXPECT_EQ(rootUrl.scheme(), "network");
}

TEST_F(UT_SmbBrowserUtils, Icon)
{
    //    EXPECT_TRUE(ins->icon().themeName() == "network-server-symbolic");
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::icon().themeName());
}

TEST_F(UT_SmbBrowserUtils, IsSmbMounted)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList(); });
    stub.set_lamda(protocol_display_utilities::getStandardSmbPaths, [] { __DBG_STUB_INVOKE__ return QStringList(); });

    EXPECT_FALSE(smb_browser_utils::isSmbMounted("smb://1.2.3.4/hello"));
    EXPECT_FALSE(smb_browser_utils::isSmbMounted("smb://1.2.3.4/itsme/"));
}

TEST_F(UT_SmbBrowserUtils, GetDeviceIdByStdSmb)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList {
                                                                       "file:///media/user/smbmounts/smb-share:host=1.2.3.4,share=hello"
                                                                   }; });

    QString (*getStdSmbPath_QString)(const QString &) = protocol_display_utilities::getStandardSmbPath;
    stub.set_lamda(getStdSmbPath_QString, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello/"; });

    EXPECT_EQ("file:///media/user/smbmounts/smb-share:host=1.2.3.4,share=hello", smb_browser_utils::getDeviceIdByStdSmb("smb://1.2.3.4/hello"));
    EXPECT_EQ("ftp://1.2.3.4", smb_browser_utils::getDeviceIdByStdSmb("ftp://1.2.3.4"));
}

TEST_F(UT_SmbBrowserUtils, IsServiceRuninig)
{
    EXPECT_FALSE(smb_browser_utils::isServiceRuning(""));
    EXPECT_FALSE(smb_browser_utils::isServiceRuning("net"));
    EXPECT_FALSE(smb_browser_utils::isServiceRuning("dde-file-manager"));

    stub.set_lamda(&QDBusInterface::isValid, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(smb_browser_utils::isServiceRuning("smb"));
    EXPECT_FALSE(smb_browser_utils::isServiceRuning("nmb"));

    stub.reset(&QDBusInterface::isValid);
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::isServiceRuning("smb"));
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::isServiceRuning("nmb"));
}

TEST_F(UT_SmbBrowserUtils, StartService)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    //    stub.set_lamda(&QDBusAbstractInterface::asyncCall, [] { __DBG_STUB_INVOKE__ return QDBusPendingCall::fromError(QDBusError()); });
    typedef QDBusPendingCall (QDBusAbstractInterface::*AsyncCall)(const QString &method,
                                                                  const QVariant &arg1,
                                                                  const QVariant &arg2,
                                                                  const QVariant &arg3,
                                                                  const QVariant &arg4,
                                                                  const QVariant &arg5,
                                                                  const QVariant &arg6,
                                                                  const QVariant &arg7,
                                                                  const QVariant &arg8);
    stub.set_lamda(static_cast<AsyncCall>(&QDBusAbstractInterface::asyncCall), []() { __DBG_STUB_INVOKE__ return QDBusPendingCall::fromError(QDBusError()); });
#else
    stub.set_lamda(&QDBusAbstractInterface::doAsyncCall, [] { __DBG_STUB_INVOKE__ return QDBusPendingCall::fromError(QDBusError()); });
#endif

    stub.set_lamda(&QDBusPendingCall::waitForFinished, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QDBusPendingCall::isValid, [] { __DBG_STUB_INVOKE__ return true; });
}

TEST_F(UT_SmbBrowserUtils, CheckAndEnableService)
{
    bool serviceRunning = true;
    stub.set_lamda(smb_browser_utils::isServiceRuning, [&] { __DBG_STUB_INVOKE__ return serviceRunning; });
    EXPECT_TRUE(smb_browser_utils::checkAndEnableService("smbd"));

    serviceRunning = false;
    stub.set_lamda(smb_browser_utils::enableServiceNow, [] { return true; });
    EXPECT_TRUE(smb_browser_utils::checkAndEnableService("smb"));

    EXPECT_FALSE(smb_browser_utils::checkAndEnableService("sb"));
}

TEST_F(UT_SmbBrowserUtils, BindSetting)
{
    Application app;
    using GetOptFunc = std::function<QVariant()>;
    using SaveOptFunc = std::function<void(const QVariant &)>;
    auto accessor = static_cast<void (SettingBackend::*)(const QString &, GetOptFunc, SaveOptFunc)>(&SettingBackend::addSettingAccessor);
    stub.set_lamda(accessor, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::bindSetting());
}

TEST_F(UT_SmbBrowserUtils, NodeMutex)
{
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::nodesMutex());
}

TEST_F(UT_SmbBrowserUtils, ShareNodes)
{
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::shareNodes());
}
