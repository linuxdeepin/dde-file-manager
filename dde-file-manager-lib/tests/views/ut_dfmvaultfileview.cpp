#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "views/dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultremovepages.h"


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
    auto closeWindow = [] (DUrl &url) {
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
}
