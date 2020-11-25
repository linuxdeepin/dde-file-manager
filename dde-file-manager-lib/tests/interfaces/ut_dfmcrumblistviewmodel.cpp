#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QFrame>

#include "interfaces/dfmcrumblistviewmodel.h"
DFM_USE_NAMESPACE
namespace  {
class TestDFMCrumbListviewModel : public testing::Test
{
public:
    DFMCrumbListviewModel *crumbListviewModel = nullptr;
    void SetUp() override
    {
        crumbListviewModel = new DFMCrumbListviewModel;

        std::cout << "start DFMCrumbListviewModel";
    }
    void TearDown() override
    {
        delete  crumbListviewModel;
        crumbListviewModel = nullptr;
        std::cout << "end DFMCrumbListviewModel";
    }
};
}
TEST_F(TestDFMCrumbListviewModel, test_removeAll)
{
    crumbListviewModel->removeAll();
}

TEST_F(TestDFMCrumbListviewModel, test_lastIndex)
{
    crumbListviewModel->lastIndex();
}
