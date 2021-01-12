#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QFrame>
#include "interfaces/dfmcrumbinterface.h"
#include "interfaces/dfmcrumbmanager.h"
#include "interfaces/dfmcrumbmanager.h"
#include "controllers/dfmmdcrumbcontrooler.h"
#include "controllers/dfmfilecrumbcontroller.h"
#include "dfmcrumbbar.h"


DFM_USE_NAMESPACE
namespace  {
class TestDFMCrumbInterface : public testing::Test
{
public:
    DFMCrumbInterface *crumbController = nullptr;
    void SetUp() override
    {
        DUrl newurl(DUrl::fromLocalFile("/home"));
        QWidget *qframe = new QWidget();
        DFMCrumbBar *p_tr = new DFMCrumbBar(qframe);
        crumbController = DFMCrumbManager::instance()->createControllerByUrl(newurl, p_tr);
        if (!crumbController) {
            qDebug() << "Unsupported url / scheme: " << newurl;
        }
        std::cout << "start TestDFMCrumbInterface";
    }
    void TearDown() override
    {
        crumbController->disconnect();
        crumbController->deleteLater();
        delete  crumbController;
        crumbController = nullptr;
        std::cout << "end TestDFMCrumbInterface";
    }
};
}
TEST_F(TestDFMCrumbInterface, test_processAction)
{
    crumbController->processAction(DFMCrumbInterface::EscKeyPressed);
}

TEST_F(TestDFMCrumbInterface, test_cancelCompletionListTransmission)
{
    crumbController->cancelCompletionListTransmission();
}
TEST_F(TestDFMCrumbInterface, test_requestCompletionList)
{
    DUrl url("/home");
    crumbController->requestCompletionList(url);
}
