#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


#define private public

#include <QSharedPointer>
#include <dfmevent.h>
#include <controllers/vaultcontroller.h>
#include <interfaces/dfmstandardpaths.h>
#include <io/dfilestatisticsjob.h>
#include <QTimer>

#include "controllers/vaultcontroller.h"
#include "vault/vaultglobaldefine.h"
#include "controllers/vaulterrorcode.h"


DFM_USE_NAMESPACE
namespace  {
    class TestVaultController : public testing::Test
    {
    public:
        QSharedPointer<VaultController> m_controller;

        virtual void SetUp() override
        {
            QString home = DFMStandardPaths::location(DFMStandardPaths::HomePath);
            m_controller = QSharedPointer<VaultController>(new VaultController());
            std::cout << "start TestVaultController" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestVaultController" << std::endl;
        }

        void loop()
        {
            QEventLoop loop;
            QTimer::singleShot(100, nullptr, [&loop]() {
                loop.exit();
            });
            loop.exec();
        }
    };
}


TEST_F(TestVaultController, tst_createFileInfo)
{
//    DUrl fileUrl = DUrl::fromVaultFile("/");
//    const QSharedPointer<DFMCreateFileInfoEvent> &&event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl);
//    const auto &&pt = m_controller->createFileInfo(event);

//    EXPECT_NE(nullptr, pt.data());
}

TEST_F(TestVaultController, tst_createDirIterator)
{
//    const DDirIteratorPointer &iterator = createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, event->url(), event->nameFilters(),
//                                                                                                    event->filters(), event->flags()));
}

TEST_F(TestVaultController, tst_createFileWatcher)
{
    QSharedPointer<DFMCreateFileWatcherEvent> event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromVaultFile("/"));
    auto ret = m_controller->createFileWatcher(event);
    EXPECT_NE(nullptr, ret);
}

TEST_F(TestVaultController, tst_open_file_files)
{
    QSharedPointer<DFMOpenFileEvent> event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl::fromVaultFile("/"));
    EXPECT_NO_FATAL_FAILURE(m_controller->openFile(event));

    DUrlList urls;
    urls << DUrl::fromVaultFile("/");
    QSharedPointer<DFMOpenFilesEvent> events = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, urls);
    EXPECT_NO_FATAL_FAILURE(m_controller->openFiles(events));
}

/// --------------------------------------------------
///                  static test
/// --------------------------------------------------
TEST_F(TestVaultController, tst_makeVaultUrl)
{
    DUrl url = m_controller->makeVaultUrl("", "file");

    EXPECT_EQ(DFMVAULT_SCHEME, url.scheme());
    EXPECT_EQ("file", url.host());
}

TEST_F(TestVaultController, tst_localUrlToVault)
{
    DUrl url = m_controller->localUrlToVault(DUrl::fromVaultFile("/test/path"));

    EXPECT_FALSE(url.isValid());

    url = m_controller->localUrlToVault(VaultController::makeVaultUrl(VaultController::makeVaultLocalPath()));
    EXPECT_TRUE(url.isValid());

    EXPECT_EQ(DFMVAULT_SCHEME, url.scheme());
    EXPECT_EQ("", url.host());
}

TEST_F(TestVaultController, tst_localToVault)
{
    DUrl url = m_controller->localToVault("/test/path");

    EXPECT_FALSE(url.isValid());

    url = m_controller->localToVault(VaultController::makeVaultLocalPath());
    EXPECT_TRUE(url.isValid());

    EXPECT_EQ(DFMVAULT_SCHEME, url.scheme());
    EXPECT_EQ("", url.host());
}

TEST_F(TestVaultController, tst_vaultToLocal)
{
    QString url = m_controller->vaultToLocal(VaultController::makeVaultUrl());
    EXPECT_FALSE(url.isEmpty());

    DUrl videoUrl = DUrl::fromComputerFile("Videos");
    url = m_controller->vaultToLocal(videoUrl);
    EXPECT_EQ(videoUrl.toLocalFile(), url);
}

TEST_F(TestVaultController, tst_vaultToLocalUrl)
{
    DUrl url = m_controller->vaultToLocalUrl(VaultController::makeVaultUrl());
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestVaultController, tst_vaultToLocalUrls)
{
    DUrlList urls;
    urls << m_controller->makeVaultUrl();
    DUrlList ret = m_controller->vaultToLocalUrls(urls);
    EXPECT_EQ(1, ret.size());
}

TEST_F(TestVaultController, tst_pathToVirtualPath)
{
    QString virPath = m_controller->pathToVirtualPath(VaultController::makeVaultLocalPath());
    QString head = virPath.left(8);
    EXPECT_EQ(DFMVAULT_SCHEME, head);
}

TEST_F(TestVaultController, tst_urlToVirtualUrl)
{
    QString virPath = m_controller->urlToVirtualUrl(VaultController::makeVaultLocalPath()).toString();
    QString head = virPath.left(8);
    EXPECT_EQ(DFMVAULT_SCHEME, head);
}

TEST_F(TestVaultController, tst_isBigFileDeleting)
{
    EXPECT_EQ(m_controller->m_isBigFileDeleting, m_controller->isBigFileDeleting());
}

TEST_F(TestVaultController, tst_isVaultFile)
{
    EXPECT_TRUE(m_controller->isVaultFile(m_controller->makeVaultLocalPath()));
    EXPECT_FALSE(m_controller->isVaultFile(DUrl::fromComputerFile("Videos").toString()));
}

TEST_F(TestVaultController, tst_getPermissions)
{
    QString home = DFMStandardPaths::location(DFMStandardPaths::HomePath);
    QFileDevice::Permissions permissions = m_controller->getPermissions(home);
    EXPECT_TRUE(permissions > 0);
}

TEST_F(TestVaultController, tst_state)
{
    VaultController::VaultState state = m_controller->state();
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        EXPECT_EQ(VaultController::NotAvailable, state);
    }
}

TEST_F(TestVaultController, tst_isRootDirectory)
{
    EXPECT_TRUE(m_controller->isRootDirectory(m_controller->makeVaultUrl().toString()));
    EXPECT_FALSE(m_controller->isRootDirectory(DUrl::fromComputerFile("Videos").toString()));
}

TEST_F(TestVaultController, tst_getErrorInfo)
{
    for (int i = 10; i < 34; i++) {
        QString err = m_controller->getErrorInfo(i);
        EXPECT_FALSE(err.isEmpty());
    }
}

TEST_F(TestVaultController, tst_toInternalPath)
{
    QString internalPath = m_controller->toInternalPath(m_controller->makeVaultUrl().toString());
    EXPECT_EQ(DFMVAULT_SCHEME, internalPath.left(8));
    EXPECT_TRUE(internalPath.contains("vault_unlocked"));
}

TEST_F(TestVaultController, tst_toExternalPath)
{
    QString externalPath = m_controller->toExternalPath(
                m_controller->makeVaultUrl(m_controller->makeVaultLocalPath()).toString());
    EXPECT_EQ(DFMVAULT_SCHEME, externalPath.left(8));
    EXPECT_FALSE(externalPath.contains("vault_unlocked"));
}

TEST_F(TestVaultController, tst_totalsize)
{
    EXPECT_EQ(m_controller->m_totalSize, m_controller->totalsize());
}

TEST_F(TestVaultController, tst_set_get_vaultState)
{
    m_controller->setVaultState(VaultController::Encrypted);
    EXPECT_EQ(VaultController::Encrypted, m_controller->getVaultState());
    EXPECT_EQ(m_controller->m_enVaultState, m_controller->getVaultState());
}

TEST_F(TestVaultController, tst_setBigFileIsDeleting)
{
    m_controller->setBigFileIsDeleting(true);
    EXPECT_TRUE(m_controller->m_isBigFileDeleting);

    m_controller->setBigFileIsDeleting(false);
    EXPECT_FALSE(m_controller->m_isBigFileDeleting);
}

TEST_F(TestVaultController, tst_updateFolderSizeLabel)
{
    m_controller->updateFolderSizeLabel(10);
    EXPECT_EQ(10, m_controller->m_totalSize);
}

TEST_F(TestVaultController, tst_vaultLockPath)
{
    EXPECT_TRUE(m_controller->vaultLockPath().contains(VAULT_ENCRYPY_DIR_NAME));
}

TEST_F(TestVaultController, tst_vaultUnlockPath)
{
    EXPECT_TRUE(m_controller->vaultUnlockPath().contains(VAULT_DECRYPT_DIR_NAME));
}

TEST_F(TestVaultController, tst_slotCreateVault)
{
    m_controller->slotCreateVault(static_cast<int>(ErrorCode::Success));
    EXPECT_EQ(VaultController::Unlocked, m_controller->m_enVaultState);
}

TEST_F(TestVaultController, tst_slotUnlockVault)
{
    m_controller->slotCreateVault(static_cast<int>(ErrorCode::Success));
    EXPECT_EQ(VaultController::Unlocked, m_controller->m_enVaultState);
}

TEST_F(TestVaultController, tst_slotLockVault)
{
    m_controller->slotLockVault(static_cast<int>(ErrorCode::Success));
    EXPECT_EQ(VaultController::Encrypted, m_controller->m_enVaultState);
}



