// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-disk-encrypt-entry:
//   BUG 316269 getAlgorithm failure must not fall back to default sha256 (empty TPM config)
//   BUG 376053 recovery key export (dev.mid(5) name, 0600, O_NOFOLLOW)
//   BUG 367501 global TPM config path uses per-run mkdtemp dir (no shared fixed path)
//   BUG 306835 progress dialog: close-button hide guarded once / dialog show guarded
//   BUG 373557 TPM dbus timeout constant >= 5 minutes
//   BUG 304381 overlay device ("/dev/dm-" + usec-overlay-) skips fstab parsing
//   BUG 307533 sortActions on empty menu must not crash

#include <gtest/gtest.h>

#include "stubext.h"
#include "menu/diskencryptmenuscene.h"
#include "gui/encryptprogressdialog.h"
#include "utils/encryptutils.h"
#include "events/eventshandler.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUrl>
#include <DWaterProgress>
#include <DDialog>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

// static guard for BUG 373557: the TPM dbus timeout must allow Polkit interaction
static_assert(kTPMDBusTimeoutMs >= 5 * 60 * 1000,
              "TPM dbus timeout must be at least 5 minutes (PMS 373557)");

namespace {
// Mock FileInfo feeding DeviceEncryptParam collection (BUG 304381)
class BlockMockFileInfo : public FileInfo
{
public:
    explicit BlockMockFileInfo(const QUrl &url, QVariantHash props)
        : FileInfo(url), m_props(std::move(props)) { }

    QVariantHash extraProperties() const override { return m_props; }
    void refresh() override { }
    QString displayOf(const DisPlayInfoType type) const override
    {
        if (type == DisPlayInfoType::kFileDisplayName)
            return QString("UT Test Disk");
        return FileInfo::displayOf(type);
    }

private:
    QVariantHash m_props;
};

QVariantHash baseDeviceProps(const QString &device, const QString &mpt)
{
    return {
        { "Device", device },
        { "IdType", "ext4" },
        { "MountPoint", mpt },
        { "MountPoints", QStringList { mpt } },
        { "Id", "/org/freedesktop/UDisks2/block_devices/ut-304381" }
    };
}
}   // namespace

// ---------------- BUG 316269 + BUG 304381 + BUG 307533: menu scene ----------------
class UT_PmsRegressionMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        scene = new DiskEncryptMenuScene();
        stub.set_lamda(&config_utils::enableEncrypt, []() -> bool { return true; });
    }
    void TearDown() override
    {
        stub.clear();
        delete scene;
        scene = nullptr;
    }
    stub_ext::StubExt stub;
    DiskEncryptMenuScene *scene = nullptr;
};

// Pre-fix: getAlgorithm() failure silently produced a default sha256 config; the
// fixed generateTPMConfig() must return an empty string instead.
TEST_F(UT_PmsRegressionMenuScene, BUG316269_GenerateTPMConfig_AlgoFail_ReturnsEmpty)
{
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *, QString *, QString *, QString *,
                      QString *, QString *, QString *, QString *) -> bool { return false; });
    EXPECT_TRUE(scene->generateTPMConfig().isEmpty());
}

TEST_F(UT_PmsRegressionMenuScene, BUG316269_GenerateTPMConfig_AlgoSuccess_ParamsKept)
{
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *h, QString *k, QString *ph, QString *pk,
                      QString *mh, QString *mk, QString *pcr, QString *bank) -> bool {
                       *h = "sha256";
                       *k = "aes256";
                       *ph = "sha256";
                       *pk = "rsa2048";
                       *mh = "sha256";
                       *mk = "ecc";
                       *pcr = "7";
                       *bank = "sha256";
                       return true;
                   });
    const QString cfg = scene->generateTPMConfig();
    ASSERT_FALSE(cfg.isEmpty());
    const QJsonObject obj = QJsonDocument::fromJson(cfg.toUtf8()).object();
    EXPECT_EQ(QString("1"), obj.value("keyslot").toString());
    EXPECT_EQ(QString("sha256"), obj.value("session-hash-alg").toString());
    EXPECT_EQ(QString("7"), obj.value("pcr").toString());
}

void stubInitDeps(stub_ext::StubExt &stub, const QVariantHash &props)
{
    stub.set_lamda(&EventsHandler::deviceEncryptStatus, [](EventsHandler *, const QString &) -> int { return 0; });
    stub.set_lamda(&EventsHandler::holderDevice, [](EventsHandler *, const QString &) -> QString { return QString(); });
    stub.set_lamda(&EventsHandler::unfinishedDecryptJob, [](EventsHandler *) -> QString { return QString(); });
    stub.set_lamda(&device_utils::encKeyType, [](const QString &) -> int { return 0; });

    using CreateFileInfoFunc = QSharedPointer<FileInfo> (*)(const QUrl &, Global::CreateFileInfoType, QString *);
    stub.set_lamda(static_cast<CreateFileInfoFunc>(&InfoFactory::create<FileInfo>),
                   [&props](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       return QSharedPointer<FileInfo>(new BlockMockFileInfo(url, props));
                   });
}

// Overlay device with an invalid (non-json) Configuration must still initialize
// successfully as TypeOverlay; pre-fix it returned false.
TEST_F(UT_PmsRegressionMenuScene, BUG304381_Initialize_OverlayDevice_SucceedsAsOverlay)
{
    QVariantHash props = baseDeviceProps("/dev/dm-0", "/media/ut-overlay-304381");
    props.insert("Symlinks", QStringList { "usec-overlay-ut" });
    props.insert("Configuration", QString("this is not json"));
    stubInitDeps(stub, props);

    const QVariantHash params { { MenuParamKey::kSelectFiles,
                                  QVariant::fromValue(QList<QUrl> { QUrl("entry:///ut-304381.blockdev") }) } };
    EXPECT_TRUE(scene->initialize(params));
    EXPECT_EQ(job_type::TypeOverlay, scene->param.jobType);
}

// Non-overlay device with invalid configuration fails initialization.
TEST_F(UT_PmsRegressionMenuScene, BUG304381_Initialize_NonOverlay_InvalidConfig_Fails)
{
    QVariantHash props = baseDeviceProps("/dev/ut-sdb5-304381", "/media/ut-disk-304381");
    props.insert("Symlinks", QStringList { "ut-disk" });
    props.insert("Configuration", QString("still not json"));
    stubInitDeps(stub, props);

    const QVariantHash params { { MenuParamKey::kSelectFiles,
                                  QVariant::fromValue(QList<QUrl> { QUrl("entry:///ut-304381.blockdev") }) } };
    EXPECT_FALSE(scene->initialize(params));
}

// Regression: sortActions used to call acts.last() on an empty menu (crash in Debug).
TEST_F(UT_PmsRegressionMenuScene, BUG307533_SortActions_EmptyMenu_NoCrash)
{
    QMenu menu;
    scene->create(&menu);   // create() only registers scene actions, menu stays empty
    EXPECT_TRUE(menu.actions().isEmpty());
    EXPECT_NO_FATAL_FAILURE(scene->sortActions(&menu));
    EXPECT_TRUE(menu.actions().isEmpty());
}

// With a pre-existing action, sortActions inserts the scene's actions before it,
// keeping the pre-existing action last (matches "insert below computer-rename").
TEST_F(UT_PmsRegressionMenuScene, BUG307533_SortActions_KeepsExistingActionLast)
{
    QMenu menu;
    scene->create(&menu);
    QAction own("own");
    menu.addAction(&own);
    scene->sortActions(&menu);
    // 6 scene actions inserted before `own`; the existing action stays last
    EXPECT_EQ(7, menu.actions().size());
    EXPECT_EQ(&own, menu.actions().last());
}

// ---------------- BUG 376053 + BUG 306835: events handler ----------------
class UT_PmsRegressionEventsHandler : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        handler = EventsHandler::instance();
        ASSERT_NE(handler, nullptr);
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
    EventsHandler *handler = nullptr;
};

TEST_F(UT_PmsRegressionEventsHandler, BUG376053_SaveRecoveryKey_WritesKeyWithStrictPermissions)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString dev = "/dev/ut-sdb1";

    EXPECT_TRUE(handler->saveRecoveryKeyToFile("ut-rec-key-123", dev, dir.path()));

    const QString expected = dir.path() + "/ut-sdb1_recovery_key.txt";   // dev.mid(5)
    QFile f(expected);
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString("ut-rec-key-123"), QString::fromUtf8(f.readAll()));
    f.close();

    const QFile::Permissions perms = QFile::permissions(expected);
    EXPECT_TRUE(perms & QFileDevice::ReadOwner);
    EXPECT_TRUE(perms & QFileDevice::WriteOwner);
    EXPECT_FALSE(perms & QFileDevice::ReadGroup);
    EXPECT_FALSE(perms & QFileDevice::WriteGroup);
    EXPECT_FALSE(perms & QFileDevice::ReadOther);
    EXPECT_FALSE(perms & QFileDevice::WriteOther);
}

// O_NOFOLLOW: a pre-existing symlink as the target must be rejected, victim untouched.
TEST_F(UT_PmsRegressionEventsHandler, BUG376053_SaveRecoveryKey_SymlinkTarget_Rejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString victim = dir.path() + "/victim.txt";
    {
        QFile v(victim);
        ASSERT_TRUE(v.open(QIODevice::WriteOnly));
        v.write("DONOTTOUCH");
    }
    QFile::link(victim, dir.path() + "/ut-symlink_recovery_key.txt");

    EXPECT_FALSE(handler->saveRecoveryKeyToFile("secret", "/dev/ut-symlink", dir.path()));

    QFile v(victim);
    ASSERT_TRUE(v.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString("DONOTTOUCH"), QString::fromUtf8(v.readAll()));
}

TEST_F(UT_PmsRegressionEventsHandler, BUG376053_SaveRecoveryKey_InvalidDir_ReturnsFalse)
{
    EXPECT_FALSE(handler->saveRecoveryKeyToFile("k", "/dev/ut-x", "/nonexistent-ut-dir-376053"));
}

// The close button must be hidden at most once per dialog lifetime.
TEST_F(UT_PmsRegressionEventsHandler, BUG306835_OnEncryptProgress_SecondCallDoesNotReshow)
{
    const QString dev = "/dev/ut-306835-unique";
    handler->onEncryptProgress(dev, "UT Disk", 0.1);   // creates + shows the dialog

    int showCount = 0;
    stub.set_lamda(&QWidget::show, [&showCount](QWidget *) { ++showCount; });

    handler->onEncryptProgress(dev, "UT Disk", 0.5);   // second progress event
    EXPECT_EQ(0, showCount);   // fix: dlg->show() guarded by dlg->isVisible()
}

// ---------------- BUG 306835 (dialog part): encrypt progress dialog ----------------
class UT_PmsRegressionProgressDialog : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(UT_PmsRegressionProgressDialog, BUG306835_UpdateProgress_HidesCloseButtonOnce)
{
    EncryptProgressDialog dlg;
    bool closeButtonHidden = false;
    int hideCalls = 0;
    stub.set_lamda(static_cast<bool (DDialog::*)() const>(&DDialog::closeButtonVisible),
                   [&closeButtonHidden](DDialog *) -> bool { return !closeButtonHidden; });
    stub.set_lamda(static_cast<void (DDialog::*)(bool)>(&DDialog::setCloseButtonVisible),
                   [&closeButtonHidden, &hideCalls](DDialog *, bool v) {
                       closeButtonHidden = !v;   // v==false means "hidden"
                       ++hideCalls;
                   });

    dlg.updateProgress(0.5);
    EXPECT_EQ(49, dlg.findChild<DWaterProgress *>()->value());
    dlg.updateProgress(0.5);
    EXPECT_EQ(1, hideCalls);   // pre-fix: hidden again on every call
    dlg.updateProgress(0.0);
    EXPECT_EQ(0, dlg.findChild<DWaterProgress *>()->value());
}

// ---------------- BUG 367501 + BUG 373557: encrypt utils ----------------
class UT_PmsRegressionEncryptUtils : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(UT_PmsRegressionEncryptUtils, BUG367501_GetGlobalTPMConfigPath_UsesRandomTempDir)
{
    const QString p1 = tpm_passphrase_utils::getGlobalTPMConfigPath();
    EXPECT_FALSE(p1.isEmpty());
    EXPECT_TRUE(QDir(p1).exists());
    EXPECT_NE(QString("/tmp/dfm-encrypt"), p1);   // fixed shared path removed by the fix
    EXPECT_TRUE(p1.startsWith(QDir::tempPath()));
    EXPECT_EQ(p1, tpm_passphrase_utils::getGlobalTPMConfigPath());   // cached for the process
}

TEST_F(UT_PmsRegressionEncryptUtils, BUG373557_TpmDBusTimeout_AtLeastFiveMinutes)
{
    EXPECT_GE(kTPMDBusTimeoutMs, 5 * 60 * 1000);
}
