#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public
#include "views/dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultremovepages.h"
#include "stub.h"
#include "dfmevent.h"


namespace  {
class TestDFMVaultFileView : public testing::Test
{
public:
    QSharedPointer<DFMVaultFileView> m_view;

    virtual void SetUp() override
    {
        m_view = QSharedPointer<DFMVaultFileView>(new DFMVaultFileView());
        m_view->show();
        std::cout << "start TestDFMVaultFileView" << std::endl;
    }

    virtual void TearDown() override
    {
        m_view->close();
        std::cout << "end TestDFMVaultFileView" << std::endl;
    }
};
}


TEST_F(TestDFMVaultFileView, tst_setRootUrl)
{
    auto closeWindow = [](DUrl & url) {
        VaultController::VaultState enState = VaultController::ins()->state();
        if (enState != VaultController::Unlocked) {
            switch (enState) {
            case VaultController::NotAvailable: {
                qDebug() << "cryfs not installed!";
                break;
            }
            case VaultController::NotExisted: {
                DFMVaultActiveView::getInstance()->close();
                break;
            }
            case VaultController::Encrypted: {
                if (url.host() == "certificate") {
                    DFMVaultRecoveryKeyPages::instance()->close();
                } else {
                    DFMVaultUnlockPages::instance()->close();
                }
                break;
            }
            default:
                ;
            }
        } else {
            if (url.host() == "delete") {
                DFMVaultRemovePages::instance()->close();
            }
        }
    };

    DUrl url = VaultController::makeVaultUrl();
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    closeWindow(url);

    url.setHost("delete");
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
    closeWindow(url);

    url.setHost("certificate");
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
    closeWindow(url);

    // replace VaultController::state
    VaultController::VaultState(*st_state)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::NotAvailable;
    };
    Stub stub;
    stub.set(ADDR(VaultController, state), st_state);
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
    closeWindow(url);

    stub.reset(ADDR(VaultController, state));
    url.setHost("delete");

    // replace VaultController::state
    VaultController::VaultState(*st_state_unLocked)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::Unlocked;
    };
    stub.set(ADDR(VaultController, state), st_state_unLocked);

    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
    closeWindow(url);

    // replace VaultController::state
    VaultController::VaultState(*st_state_encrypted)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::Encrypted;
    };
    stub.set(ADDR(VaultController, state), st_state_encrypted);

    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
    closeWindow(url);
}

TEST_F(TestDFMVaultFileView, tst_onLeaveVault)
{
    EXPECT_NO_FATAL_FAILURE(m_view->onLeaveVault(0));
}

TEST_F(TestDFMVaultFileView, tst_eventFilter)
{
    QSharedPointer<QMouseEvent> event = dMakeEventPointer<QMouseEvent>(
                                            QMouseEvent::KeyPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    m_view->eventFilter(nullptr, event.get());
}
