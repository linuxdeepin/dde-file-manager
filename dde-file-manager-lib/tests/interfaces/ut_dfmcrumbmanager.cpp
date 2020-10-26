#include "interfaces/dfmcrumbmanager.h"
#include "controllers/dfmmdcrumbcontrooler.h"
#include "controllers/dfmfilecrumbcontroller.h"
#include <gtest/gtest.h>
#include <QFrame>
#include "dfmcrumbbar.h"
DFM_USE_NAMESPACE

namespace {
class TestDFMCrumbManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMCrumbManager";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMCrumbManager";
    }
};
}

TEST_F(TestDFMCrumbManager, isRegisted)
{
    DFMCrumbManager::instance()->dRegisterCrumbCreator<DFMFileCrumbController>(QStringLiteral(FILE_SCHEME));
    EXPECT_TRUE(DFMCrumbManager::instance()->isRegisted<DFMFileCrumbController>(QStringLiteral(FILE_SCHEME)));
}

TEST_F(TestDFMCrumbManager, createControllerByUrl)
{
    DUrl newurl(DUrl::fromLocalFile("/home"));
    DFMCrumbInterface *crumbController = nullptr;
    QWidget *qframe = new QWidget();
    DFMCrumbBar *p_tr = new DFMCrumbBar(qframe);
    crumbController = DFMCrumbManager::instance()->createControllerByUrl(newurl, p_tr);
    EXPECT_TRUE(crumbController != nullptr);
}

