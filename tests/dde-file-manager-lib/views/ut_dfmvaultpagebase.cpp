/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "dfmglobal.h"
#include "controllers/appcontroller.h"

#include <QWindow>
#include "stub.h"

#define private public
#include "views/dfmvaultactiveview.h"


namespace  {
class TestDFMVaultPageBase : public testing::Test
{
public:
    QSharedPointer<DFMVaultPageBase> m_view;

    virtual void SetUp() override
    {
        m_view = QSharedPointer<DFMVaultPageBase>(new DFMVaultPageBase());
        m_view->show();
        std::cout << "start TestDFMVaultPageBase" << std::endl;
    }

    virtual void TearDown() override
    {
        m_view->close();
        std::cout << "end TestDFMVaultPageBase" << std::endl;
    }
};
}


TEST_F(TestDFMVaultPageBase, tst_set_get_wndPtr)
{
    QSharedPointer<QWidget> window = QSharedPointer<QWidget>(new QWidget());
    m_view->setWndPtr(window.get());
    EXPECT_EQ(window, m_view->getWndPtr());
}

TEST_F(TestDFMVaultPageBase, tst_showTop)
{
    m_view->showTop();
    EXPECT_FALSE(m_view->isHidden());
    EXPECT_TRUE(m_view->isTopLevel());
}

TEST_F(TestDFMVaultPageBase, tst_contruct)
{
    bool (*st_isWayland)() = []()->bool {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFMGlobal, isWayLand), st_isWayland);

    DFMVaultPageBase *pageBase = new DFMVaultPageBase();
    delete pageBase;
    pageBase = nullptr;
}

TEST_F(TestDFMVaultPageBase, tst_enterVaultDir)
{
    void (*st_actionOpen)(const QSharedPointer<DFMUrlListBaseEvent> &, const bool) =
            [](const QSharedPointer<DFMUrlListBaseEvent> &, const bool)->void {
        // do nothing.
    };
    Stub stub;
    stub.set(ADDR(AppController, actionOpen), st_actionOpen);

    m_view->enterVaultDir();
}
