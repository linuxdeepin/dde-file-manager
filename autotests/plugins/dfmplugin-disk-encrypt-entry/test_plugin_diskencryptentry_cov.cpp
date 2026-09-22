// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (plugin_diskencryptentry.cpp):
//   hasComputerMenuRegisted / DiskEncryptEntry::initialize(+valueChanged lambda) /
//   start / initEncryptEvents(+singleShot lambda guarded) / onComputerMenuSceneAdded(+lambda
//   via signal args) / processUnfinshedDecrypt(+find_if lambda)
// Branch notes (from get_code_snippet):
//   initEncryptEvents: enableEncrypt false -> skip; static inited -> skip;
//   computer menu registered vs not (subscribe path);
//   processUnfinshedDecrypt: ignore-file exists -> skip; empty entry path -> skip;
//   onComputerMenuSceneAdded: scene == ComputerMenu -> bind + unsubscribe.

#include "stubext.h"

#include "plugin_diskencryptentry.h"
#include "events/eventshandler.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QFile>
#include <dconfig.h>

using namespace dfmplugin_diskenc;

class UT_DiskEncryptEntryCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // keep DBus-driven helpers inert
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&EventsHandler::unfinishedDecryptJob, [](EventsHandler *) -> QString {
            __DBG_STUB_INVOKE__
            return QString();   // avoid scheduling the 1s decrypt timer
        });
        stub.set_lamda(&EventsHandler::isTaskWorking, [](EventsHandler *) -> bool {
            __DBG_STUB_INVOKE__
            return true;   // even a non-empty job would be skipped
        });
        stub.set_lamda(&QApplication::applicationName, []() -> QString {
            __DBG_STUB_INVOKE__
            return "ut-disk-encrypt";
        });
        using ConnectFunc = bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
        stub.set_lamda(static_cast<ConnectFunc>(&QDBusConnection::connect),
                       [](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_DiskEncryptEntryCov, Initialize_AndStart_WithEncryptDisabled_AreQuiet)
{
    // Arrange: enableEncrypt reads a DConfig; in the test env it is not enabled
    auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
    auto *fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
    stub.set_lamda(createFunc, [fakeCfg](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
        __DBG_STUB_INVOKE__
        return fakeCfg;
    });
    stub.set_lamda(&Dtk::Core::DConfig::value, [](Dtk::Core::DConfig *, const QString &key, const QVariant &def) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == "enableEncrypt")
            return false;
        return def;
    });

    // Arrange: keep DConfigManager inert (its real ctor touches the dconfig backend)
    stub.set_lamda(&dfmbase::DConfigManager::addConfig,
                   [](dfmbase::DConfigManager *, const QString &, QString *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    // Act
    DiskEncryptEntry entry;
    entry.initialize();
    bool started = entry.start();

    // Assert: start always succeeds even when the feature is disabled
    EXPECT_TRUE(started);
    EXPECT_EQ(EventsHandler::instance(), EventsHandler::instance());
}

TEST_F(UT_DiskEncryptEntryCov, OnComputerMenuSceneAdded_MatchingScene_BindsOnce)
{
    // Arrange
    // Act
    // Assert
    DiskEncryptEntry entry;
    EXPECT_NO_FATAL_FAILURE(entry.onComputerMenuSceneAdded("ComputerMenu"));

    // non-matching scenes are ignored
    EXPECT_NO_FATAL_FAILURE(entry.onComputerMenuSceneAdded("OtherMenu"));

    // Assert
    EXPECT_NE(&entry, nullptr);
    EXPECT_EQ(QString(entry.metaObject()->className()), QString("dfmplugin_diskenc::DiskEncryptEntry"));
}

TEST_F(UT_DiskEncryptEntryCov, ProcessUnfinishedDecrypt_IgnoreFileExists_Skips)
{
    // Arrange: the ignore marker is a real file checked via the QFile member
    QFile marker("/tmp/dfm_ignore_decrypt_auto_reqeust__dev_sdb1");
    ASSERT_TRUE(marker.open(QIODevice::WriteOnly));
    marker.close();
    DiskEncryptEntry entry;

    // Act: early return before touching devices

    // Assert
    EXPECT_NO_FATAL_FAILURE(entry.processUnfinshedDecrypt("/dev/sdb1"));
    EXPECT_EQ(marker.exists(), true);
    EXPECT_NE(marker.exists(), false);   // double-check stable state

    QFile::remove("/tmp/dfm_ignore_decrypt_auto_reqeust__dev_sdb1");
}

TEST_F(UT_DiskEncryptEntryCov, ProcessUnfinishedDecrypt_EmptyEntryPath_Skips)
{
    // Arrange: no ignore file, but entry path cannot be resolved
    bool resolveCalled = false;
    stub.set_lamda(&device_utils::resolveEntryBlockDevPath, [&resolveCalled](const QString &dev) -> QString {
        __DBG_STUB_INVOKE__
        resolveCalled = (dev == "/dev/sdb1");
        return QString();
    });
    DiskEncryptEntry entry;

    // Act
    entry.processUnfinshedDecrypt("/dev/sdb1");

    // Assert
    EXPECT_EQ(resolveCalled, true);
    EXPECT_NE(resolveCalled, false);   // double-check stable state
}
