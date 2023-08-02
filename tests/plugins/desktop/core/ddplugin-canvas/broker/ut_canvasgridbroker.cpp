// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "broker/canvasgridbroker.h"
#include "grid/canvasgrid.h"
#include "stubext.h"

#include <gtest/gtest.h>
using namespace ddplugin_canvas;

TEST(CanvasGridBroker,point)
{
   CanvasGrid grid;
   CanvasGridBroker broker(&grid);
   QPoint pos(1,1);
   EXPECT_EQ(broker.point(QString("temp_str"),&pos),-1);
}
TEST(CanvasGridBroker,tryAppendAfter)
{
    stub_ext::StubExt stub;
    bool call = false;
    stub.set_lamda(&CanvasGrid::tryAppendAfter,[&call](CanvasGrid*,const QStringList &items, int index, const QPoint &begin){
        call = true;
    });
    CanvasGrid grid;
    CanvasGridBroker broker(&grid);
    QPoint pos(1,1);
    QString str = QString("temp_str");

    broker.tryAppendAfter(QStringList{str},1,pos);
    EXPECT_TRUE(call);
}
