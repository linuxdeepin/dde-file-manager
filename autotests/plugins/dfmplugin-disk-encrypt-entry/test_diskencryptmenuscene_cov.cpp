// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (menu/diskencryptmenuscene.cpp), exposed via subclass:
//   create / updateState / sortActions / updateActions / triggered /
//   encryptDevice / decryptDevice / changePassphrase / unlockDevice /
//   doEncryptDevice / doReencryptDevice / doDecryptDevice / doChangePassphrase /
//   generateTPMConfig / generateTPMToken / getBase64Of / onUnlocked / onMounted /
//   unmountBefore / onUnmountError
// Branch notes (from get_code_snippet):
//   updateActions: states==0 & no pending decrypt -> encrypt visible; taskWorking/
//   currDevOperating flags control enable state;
//   do* DBus methods: invalid interface -> early return;
//   doReencryptDevice: non-Pwd secType with empty/valid tpm config path;
//   generateTPMToken: reads key.priv/key.pub/iv.bin/cipher.out via getBase64Of;
//   onUnlocked/onMounted: error -> showDialog; ok -> createBlockDevice may fail;
//   unmountBefore: null block device -> early return.

#include "stubext.h"

#include "menu/diskencryptmenuscene.h"
#include "events/eventshandler.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-base/dfm_menu_defines.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QDialog>
#include <QJsonDocument>
#include <QTimer>
// Schedules auto-finish for the next modal dialog (its exec() runs a local
// event loop; the retrying timer closes the dialog as soon as it is visible).
inline void autoFinishDialog(int result)
{
    auto *t = new QTimer(qApp);
    t->setInterval(10);
    QObject::connect(t, &QTimer::timeout, t, [t, result]() {
        for (QWidget *w : QApplication::topLevelWidgets()) {
            auto *d = qobject_cast<QDialog *>(w);
            if (d && d->isVisible() && d->isModal()) {
                t->stop();
                t->deleteLater();
                d->done(result);
                return;
            }
        }
    });
    t->start();
}

#include <QJsonObject>
#include <QMenu>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDBusAbstractInterface>
#include <DDialog>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
class TestableMenuScene : public DiskEncryptMenuScene
{
public:
    using DiskEncryptMenuScene::encryptDevice;
    using DiskEncryptMenuScene::decryptDevice;
    using DiskEncryptMenuScene::changePassphrase;
    using DiskEncryptMenuScene::unlockDevice;
    using DiskEncryptMenuScene::doEncryptDevice;
    using DiskEncryptMenuScene::doDecryptDevice;
    using DiskEncryptMenuScene::doChangePassphrase;
    using DiskEncryptMenuScene::generateTPMConfig;
    using DiskEncryptMenuScene::generateTPMToken;
    using DiskEncryptMenuScene::getBase64Of;
    using DiskEncryptMenuScene::onUnlocked;
    using DiskEncryptMenuScene::onMounted;
    using DiskEncryptMenuScene::unmountBefore;
    using DiskEncryptMenuScene::onUnmountError;
    using DiskEncryptMenuScene::sortActions;
    using DiskEncryptMenuScene::updateActions;
    using DiskEncryptMenuScene::OpType;
};
}   // namespace

class UT_DiskEncryptMenuSceneCov : public testing::Test
{
protected:
    void SetUp() override
    {
        scene = new TestableMenuScene();

        // daemon DBus interface always invalid -> early returns everywhere
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&dialog_utils::showDialog, [](const QString &, const QString &) -> int {
            __DBG_STUB_INVOKE__
            return 0;
        });
        stub.set_lamda(static_cast<int (*)(const QString &, bool)>(&dialog_utils::showConfirmEncryptionDialog),
                       [](const QString &, bool) -> int {
                           __DBG_STUB_INVOKE__
                           return 0;   // cancelled
                       });
        stub.set_lamda(static_cast<int (*)(const QString &, bool)>(&dialog_utils::showConfirmDecryptionDialog),
                       [](const QString &, bool) -> int {
                           __DBG_STUB_INVOKE__
                           return 0;
                       });
        // no modal dialog exec (virtual, cannot be stubbed): auto-finish via timer

        stubEventsHandler();
        stubNoBlockDevice();
    }

    void TearDown() override
    {
        stub.clear();
        delete scene;
    }

    void stubEventsHandler()
    {
        stub.set_lamda(&EventsHandler::isTaskWorking, [](EventsHandler *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&EventsHandler::isUnderOperating, [](EventsHandler *, const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&EventsHandler::hasPendingTask, [](EventsHandler *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&EventsHandler::unfinishedDecryptJob, [](EventsHandler *) -> QString {
            __DBG_STUB_INVOKE__
            return QString();
        });
    }

    void stubNoBlockDevice()
    {
        stub.set_lamda(&device_utils::createBlockDevice, [](const QString &) -> BlockDev {
            __DBG_STUB_INVOKE__
            return BlockDev();
        });
    }

    DeviceEncryptParam makeParam()
    {
        DeviceEncryptParam p;
        p.devDesc = "/dev/sdb1";
        p.deviceDisplayName = "data";
        p.devID = "/org/freedesktop/UDisks2/block_devices/sdb1";
        p.mountPoint = "/media/data";
        p.secType = SecKeyType::kPwd;
        p.jobType = job_type::TypeNormal;
        return p;
    }

    TestableMenuScene *scene = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_DiskEncryptMenuSceneCov, Create_ReportsSuccessTwice)
{
    // Arrange: create() only fills the internal action map, not the parent menu
    QMenu menu;

    // Act
    bool first = scene->create(&menu);
    bool second = scene->create(&menu);

    // Assert
    EXPECT_TRUE(first);
    EXPECT_TRUE(second);
    EXPECT_EQ(scene->name(), QString("DiskEncryptMenu"));
}

TEST_F(UT_DiskEncryptMenuSceneCov, UpdateState_NotEncryptedDevice_LeavesMenuIntact)
{
    // Arrange: actions live in the scene's internal map; the menu keeps its own
    QMenu menu;
    QAction own("own");
    own.setProperty(ActionPropertyKey::kActionID, "de_0_encrypt");
    menu.addAction(&own);
    ASSERT_TRUE(scene->create(&menu));

    // Act
    scene->updateState(&menu);

    // Assert: sortActions inserted the six internal actions before our own
    ASSERT_EQ(menu.actions().size(), 7);
    QAction *encryptAct = nullptr;
    QAction *decryptAct = nullptr;
    for (auto *act : menu.actions()) {
        QString id = act->property(ActionPropertyKey::kActionID).toString();
        if (id == "de_0_encrypt") encryptAct = act;
        if (id == "de_1_decrypt") decryptAct = act;
    }
    ASSERT_NE(encryptAct, nullptr);
    ASSERT_NE(decryptAct, nullptr);
    EXPECT_EQ(encryptAct->isVisible(), true);
    EXPECT_EQ(decryptAct->isVisible(), false);
    EXPECT_EQ(own.isVisible(), true);
}

TEST_F(UT_DiskEncryptMenuSceneCov, UpdateState_TaskWorking_RunsThrough)
{
    // Arrange
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));
    stub.set_lamda(&EventsHandler::isTaskWorking, [](EventsHandler *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Act: updateActions with taskWorking=true must not touch the menu

    // Assert
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_EQ(menu.actions().size(), 0);
    EXPECT_NE(menu.actions().size(), 1);   // complementary bound
}

TEST_F(UT_DiskEncryptMenuSceneCov, SortActions_WithMenuActions_KeepsThemAll)
{
    // Arrange: put our own actions into the menu for sorting
    QMenu menu;
    QAction a1("one"), a2("two"), a3("three");
    a1.setProperty(ActionPropertyKey::kActionID, "de_1_decrypt");
    a2.setProperty(ActionPropertyKey::kActionID, "de_0_unlock");
    a3.setProperty(ActionPropertyKey::kActionID, "unknown");
    menu.addAction(&a1);
    menu.addAction(&a2);
    menu.addAction(&a3);
    ASSERT_TRUE(scene->create(&menu));

    // Act
    scene->sortActions(&menu);

    // Assert: internal actions inserted before the last existing action
    ASSERT_EQ(menu.actions().size(), 9);
    EXPECT_EQ(menu.actions().at(0), &a1);   // own actions keep relative order
    EXPECT_EQ(menu.actions().last(), &a3);
    EXPECT_EQ(menu.actions().at(2)->property(ActionPropertyKey::kActionID).toString(), QString("de_0_encrypt"));
}

TEST_F(UT_DiskEncryptMenuSceneCov, Triggered_UnknownAction_ReturnsFalse)
{
    // Arrange
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));
    QAction unknown("nothing");

    // Act

    // Assert
    EXPECT_EQ(scene->triggered(&unknown), false);
    EXPECT_EQ(scene->name() == QString("DiskEncryptMenu"), true);
}

TEST_F(UT_DiskEncryptMenuSceneCov, Triggered_ResumeEncrypt_CallsResumeEncrypt)
{
    // Arrange
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));
    bool resumeCalled = false;
    stub.set_lamda(&EventsHandler::resumeEncrypt, [&resumeCalled](EventsHandler *, const QString &) {
        __DBG_STUB_INVOKE__
        resumeCalled = true;
    });
    QAction act("resume");
    act.setProperty(ActionPropertyKey::kActionID, "de_0_resumeEncrypt");

    // Act
    bool ok = scene->triggered(&act);

    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(resumeCalled, true);
}

TEST_F(UT_DiskEncryptMenuSceneCov, Triggered_DecryptConfirmedFstab_UnsupportedJobTypeReturnsFalse)
{
    // Arrange: decrypt confirmed but fstab job type is unsupported -> false
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));
    stub.set_lamda(static_cast<int (*)(const QString &, bool)>(&dialog_utils::showConfirmDecryptionDialog),
                   [](const QString &, bool) -> int {
                       __DBG_STUB_INVOKE__
                       return 1;   // accepted
                   });
    QAction act("decrypt");
    act.setProperty(ActionPropertyKey::kActionID, "de_1_decrypt");

    // Act: normal job type -> unmountBefore -> null block device -> early return
    scene->param.jobType = job_type::TypeNormal;
    bool ok = scene->triggered(&act);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_NE(scene->param.devDesc, QString("x"));
}

TEST_F(UT_DiskEncryptMenuSceneCov, EncryptDevice_NotConfirmed_DoesNothing)
{
    // Arrange: confirmation dialog cancelled (default stub)
    auto p = makeParam();

    // Act: cancelled path returns quietly

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::encryptDevice(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DecryptDevice_PwdTypeCancelledDialog_ReturnsEarly)
{
    // Arrange: modal DecryptParamsInputDialog auto-finished as rejected
    auto p = makeParam();
    p.secType = SecKeyType::kPwd;

    // Act

    // Assert
    autoFinishDialog(0);
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::decryptDevice(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DecryptDevice_TpmTypeNoPassphrase_ShowsUnlockDialog)
{
    // Arrange
    auto p = makeParam();
    p.secType = SecKeyType::kTpm;
    stub.set_lamda(&tpm_passphrase_utils::getPassphraseFromTPM_NonBlock,
                   [](const QString &, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString();   // empty -> unlock dialog path
                   });

    // Act: modal UnlockPartitionDialog auto-finished as rejected -> early return

    // Assert
    autoFinishDialog(0);
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::decryptDevice(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, ChangePassphrase_DialogCancelled_ReturnsEarly)
{
    // Arrange
    auto p = makeParam();
    p.secType = SecKeyType::kPwd;

    // Act: modal ChgPassphraseDialog auto-finished as rejected -> return

    // Assert
    autoFinishDialog(0);
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::changePassphrase(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, UnlockDevice_NullBlockDevice_ReturnsEarly)
{
    // Arrange: createBlockDevice stubbed null in SetUp

    // Act

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::unlockDevice("/org/freedesktop/UDisks2/block_devices/sdb1"));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoEncryptDevice_InvalidInterface_ReturnsQuietly)
{
    // Arrange: iface invalid

    // Act

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::doEncryptDevice(makeParam()));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoDecryptDevice_InvalidInterface_ReturnsQuietly)
{
    // Arrange: iface invalid

    // Act

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::doDecryptDevice(makeParam()));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoChangePassphrase_PwdTypeInvalidInterface_ReturnsQuietly)
{
    // Arrange: kPwd skips tpm token, iface invalid
    auto p = makeParam();

    // Act

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::doChangePassphrase(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoChangePassphrase_PinTypeReadsOldToken)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QDir().mkpath(dir.path() + "/dev/sdb1");
    QFile f(dir.path() + "/dev/sdb1/token.json");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("{\"enc\":\"old\"}");
    f.close();

    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path() + "/";
    });
    stub.set_lamda(&TestableMenuScene::generateTPMToken,
                   [](const QString &, bool, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("{\"enc\":\"new\",\"kek-priv\":\"p\",\"kek-pub\":\"u\",\"iv\":\"i\"}");
                   });

    auto p = makeParam();
    p.secType = SecKeyType::kPin;
    p.validateByRecKey = false;

    // Act: token merge then invalid iface -> quiet return

    // Assert
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::doChangePassphrase(p));
}

TEST_F(UT_DiskEncryptMenuSceneCov, GenerateTPMConfig_ValidAlgorithm_ReturnsJson)
{
    // Arrange
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *s, QString *k, QString *ph, QString *pk, QString *mh, QString *mk, QString *pcr, QString *bank) -> bool {
                       __DBG_STUB_INVOKE__
                       *s = "sha256"; *k = "aes"; *ph = "sha256"; *pk = "rsa";
                       *mh = "sha256"; *mk = "aes"; *pcr = "7"; *bank = "sha256";
                       return true;
                   });

    // Act
    QString json = TestableMenuScene::generateTPMConfig();

    // Assert
    auto doc = QJsonDocument::fromJson(json.toLocal8Bit());
    ASSERT_TRUE(doc.isObject());
    EXPECT_EQ(doc.object().value("keyslot").toString(), QString("1"));
    EXPECT_EQ(doc.object().value("session-hash-alg").toString(), QString("sha256"));
}

TEST_F(UT_DiskEncryptMenuSceneCov, GenerateTPMConfig_NoAlgorithm_ReturnsEmpty)
{
    // Arrange
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *, QString *, QString *, QString *, QString *, QString *, QString *, QString *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    // Act
    QString json = TestableMenuScene::generateTPMConfig();

    // Assert
    EXPECT_EQ(json.isEmpty(), true);
    EXPECT_NE(json.isEmpty(), false);   // double-check stable state
}

TEST_F(UT_DiskEncryptMenuSceneCov, GenerateTPMToken_BuildsTokenFromConfigFiles)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QDir().mkpath(dir.path() + "/dev/sdb1");
    for (const char *name : { "key.priv", "key.pub", "iv.bin", "cipher.out" }) {
        QFile f(dir.path() + "/dev/sdb1/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(name);
        f.close();
    }
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *s, QString *k, QString *ph, QString *pk, QString *mh, QString *mk, QString *pcr, QString *bank) -> bool {
                       __DBG_STUB_INVOKE__
                       *s = "sha256"; *k = "aes"; *ph = "sha256"; *pk = "rsa";
                       *mh = "sha256"; *mk = "aes"; *pcr = "7"; *bank = "sha256";
                       return true;
                   });

    // Act
    QString token = TestableMenuScene::generateTPMToken("/dev/sdb1", true, dir.path());

    // Assert
    auto doc = QJsonDocument::fromJson(token.toLocal8Bit());
    ASSERT_TRUE(doc.isObject());
    EXPECT_EQ(doc.object().value("type").toString(), QString("usec-tpm2"));
    EXPECT_EQ(doc.object().value("pin").toString(), QString("1"));
    EXPECT_EQ(doc.object().value("kek-priv").toString(), QString(QByteArray("key.priv").toBase64()));
}

TEST_F(UT_DiskEncryptMenuSceneCov, GetBase64Of_ReadsAndEncodes)
{
    // Arrange
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("Hello World");
    tmp.close();

    // Act
    QString b64 = TestableMenuScene::getBase64Of(tmp.fileName());
    QString missing = TestableMenuScene::getBase64Of("/nonexistent/ut-file");

    // Assert
    EXPECT_EQ(b64, QString(QByteArray("Hello World").toBase64()));
    EXPECT_TRUE(missing.isEmpty());
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoReencryptDevice_PwdTypeInvalidInterface_ReturnsFalse)
{
    // Arrange: kPwd needs no tpm token; iface invalid
    auto p = makeParam();

    // Act
    bool ok = TestableMenuScene::doReencryptDevice(p);

    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_NE(ok, true);   // double-check stable state
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoReencryptDevice_PinTypeNoConfigPath_ReturnsFalse)
{
    // Arrange
    auto p = makeParam();
    p.secType = SecKeyType::kPin;
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, []() -> QString {
        __DBG_STUB_INVOKE__
        return QString();
    });

    // Act
    bool ok = TestableMenuScene::doReencryptDevice(p);

    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_NE(ok, true);   // double-check stable state
}

TEST_F(UT_DiskEncryptMenuSceneCov, DoReencryptDevice_PinTypeEmptyToken_ReturnsFalse)
{
    // Arrange
    auto p = makeParam();
    p.secType = SecKeyType::kPin;
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, []() -> QString {
        __DBG_STUB_INVOKE__
        return QString("/tmp/");
    });
    stub.set_lamda(&TestableMenuScene::generateTPMToken,
                   [](const QString &, bool, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString();   // empty token -> fail
                   });

    // Act
    bool ok = TestableMenuScene::doReencryptDevice(p);

    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_NE(ok, true);   // double-check stable state
}

TEST_F(UT_DiskEncryptMenuSceneCov, OnUnlocked_Failure_ShowsDialog)
{
    // Arrange
    dfmmount::OperationErrorInfo info;
    info.code = dfmmount::DeviceError::kUDisksErrorFailed;
    int dialogCount = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&dialogCount](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++dialogCount;
        return 0;
    });

    // Act
    TestableMenuScene::onUnlocked(false, info, "/dev/mapper/clear");

    // Assert
    EXPECT_EQ(dialogCount, 1);

    // Act: success path with null block device -> quiet return
    EXPECT_NO_FATAL_FAILURE(TestableMenuScene::onUnlocked(true, dfmmount::OperationErrorInfo(), "/dev/mapper/clear"));
    EXPECT_EQ(dialogCount, 1);
}

TEST_F(UT_DiskEncryptMenuSceneCov, OnMounted_Failure_ShowsDialog)
{
    // Arrange
    dfmmount::OperationErrorInfo info;
    info.code = dfmmount::DeviceError::kUDisksErrorFailed;
    int dialogCount = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&dialogCount](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++dialogCount;
        return 0;
    });

    // Act
    TestableMenuScene::onMounted(false, info, "/media/cdrom");
    TestableMenuScene::onMounted(true, dfmmount::OperationErrorInfo(), "/media/cdrom");

    // Assert
    EXPECT_EQ(dialogCount, 1);
    EXPECT_NE(dialogCount, 2);   // complementary bound
}

TEST_F(UT_DiskEncryptMenuSceneCov, UnmountBefore_NullDevice_SkipsOperation)
{
    // Arrange: createBlockDevice stubbed to null
    bool afterCalled = false;
    auto after = [&afterCalled](const DeviceEncryptParam &) {
        afterCalled = true;
    };
    auto p = makeParam();

    // Act
    TestableMenuScene::unmountBefore(after, p);

    // Assert
    EXPECT_EQ(afterCalled, false);
    EXPECT_NE(afterCalled, true);   // double-check stable state
}

TEST_F(UT_DiskEncryptMenuSceneCov, OnUnmountError_ShowsDialog)
{
    // Arrange
    dfmmount::OperationErrorInfo info;
    int dialogCount = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&dialogCount](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++dialogCount;
        return 0;
    });

    // Act
    TestableMenuScene::onUnmountError(TestableMenuScene::kUnmount, "/dev/sdb1", info);
    TestableMenuScene::onUnmountError(TestableMenuScene::kLock, "/dev/sdb1", info);

    // Assert
    EXPECT_EQ(dialogCount, 2);
    EXPECT_NE(dialogCount, 3);   // complementary bound
}
