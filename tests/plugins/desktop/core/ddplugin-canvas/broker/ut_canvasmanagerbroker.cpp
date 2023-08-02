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
    stub_ext::StubExt stub;
    CanvasManager *manager = new CanvasManager();
    CanvasManagerBroker *broker = new CanvasManagerBroker(manager);
    broker->canvas = manager;
    manager->d->hookIfs = new CanvasManagerHook;
    broker->setAutoArrange(true);
    EXPECT_TRUE(manager->autoArrange());
    EXPECT_TRUE(broker->autoArrange());
    broker->fileInfoModel();
    EXPECT_EQ(broker->view(1),nullptr);
    delete manager;
    delete broker;
}
