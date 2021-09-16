/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hujianzhong<hujianzhong@uniontech.com>
 *
 * Maintainer: hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
