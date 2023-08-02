// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "broker/canvasmanagerbroker.h"
#include "canvasmanager.h"
#include "private/canvasmanager_p.h"
#include "model/fileinfomodel.h"
#include "view/canvasview.h"

#include <dfm-framework/dpf.h>

#include <QAbstractItemView>
#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

TEST(CanvasManagerBroker,test)
{
    CanvasManager manager;
    CanvasManagerBroker broker(&manager) ;
    broker.canvas = &manager;
    CanvasManagerHook broker1;
    manager.d->hookIfs = &broker1;
    broker.setAutoArrange(true);
    EXPECT_TRUE(manager.autoArrange());
    EXPECT_TRUE(broker.autoArrange());
    broker.fileInfoModel();
    EXPECT_EQ(broker.view(1),nullptr);
}
