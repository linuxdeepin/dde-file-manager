// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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


