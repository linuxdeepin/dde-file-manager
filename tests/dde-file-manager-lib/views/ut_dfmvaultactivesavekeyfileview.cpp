#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QSharedPointer>

#include "stub.h"
#include "addr_pri.h"

#include "../views/dfmvaultactivesavekeyfileview.h"

namespace  {
    class TestDFMVaultActionSaveKeyFileView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveSaveKeyFileView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveSaveKeyFileView>( new DFMVaultActiveSaveKeyFileView());
            m_view->show();
            std::cout << "start TestDFMVaultActiveSaveKeyFileView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveSaveKeyFileView" << std::endl;
        }
    };
}

TEST_F(TestDFMVaultActionSaveKeyFileView, callslotNextBtnClicked)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotNextBtnClicked());
}

ACCESS_PRIVATE_FIELD(DFMVaultActiveSaveKeyFileView,  QRadioButton *, m_defaultPathRadioBtn)
QRadioButton * PrivatedefaultPathRadioBtn(QSharedPointer<DFMVaultActiveSaveKeyFileView> actionSaveKeyFileView)
{
    return access_private_field::DFMVaultActiveSaveKeyFileViewm_defaultPathRadioBtn(*actionSaveKeyFileView);
}

ACCESS_PRIVATE_FIELD(DFMVaultActiveSaveKeyFileView,  QRadioButton *, m_otherPathRadioBtn)
QRadioButton * PrivateotherPathRadioBtn(QSharedPointer<DFMVaultActiveSaveKeyFileView> actionSaveKeyFileView)
{
    return access_private_field::DFMVaultActiveSaveKeyFileViewm_otherPathRadioBtn(*actionSaveKeyFileView);
}

TEST_F(TestDFMVaultActionSaveKeyFileView, callslotSelectRadioBtn)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotSelectRadioBtn(PrivatedefaultPathRadioBtn(m_view)));
    EXPECT_NO_FATAL_FAILURE(m_view->slotSelectRadioBtn(PrivateotherPathRadioBtn(m_view)));
}
