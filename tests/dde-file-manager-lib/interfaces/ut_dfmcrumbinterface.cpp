// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "interfaces/dfmcrumbinterface.h"
#include "interfaces/dfmcrumbmanager.h"
#include "interfaces/dfmcrumbmanager.h"
#include "controllers/dfmmdcrumbcontrooler.h"
#include "controllers/dfmfilecrumbcontroller.h"
#include "dfmcrumbbar.h"
#include "testhelper.h"

#include <QFrame>
#include <QThreadPool>

DFM_USE_NAMESPACE
namespace  {
class TestDFMCrumbInterface : public testing::Test
{
public:
    DFMCrumbInterface *crumbController = nullptr;
    DFMCrumbBar *p_tr=nullptr;
     QWidget *qframe=nullptr;
    void SetUp() override
    {
        DUrl newurl(DUrl::fromLocalFile("/home"));
        qframe = new QWidget();
        p_tr = new DFMCrumbBar(qframe);
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
        p_tr->deleteLater();
        qframe->deleteLater();
        TestHelper::runInLoop([](){}, 50);
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
    QThreadPool::globalInstance()->waitForDone(60*1000);
    //等待一分钟等待线程执行完毕
}
