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

#include <QAbstractItemModel>
#include <QMouseEvent>
#include "models/computermodel.h"
#include "views/computerview.h"

#include "views/computerviewitemdelegate.h"
#include "dfmevent.h"
#include "views/dfilemanagerwindow.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

DFM_USE_NAMESPACE
namespace  {
class TestFileSignalManager : public testing::Test
{
public:
    FileSignalManager *fsm;

    virtual void SetUp() override
    {
        fsm = new FileSignalManager() ;
    }

    virtual void TearDown() override
    {
        fsm->deleteLater();
    }
};
}


TEST_F(TestFileSignalManager, tst_test)
{

}


