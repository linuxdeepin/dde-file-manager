// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "broker/canvasmodelbroker.h"
#include "model/canvasproxymodel.h"
#include "model/canvasproxymodel_p.h"
#include <dfm-framework/dpf.h>

using namespace ddplugin_canvas;

#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

TEST(CanvasModelBroker,test)
{
    CanvasProxyModel proxy;
    CanvasModelBroker broker(&proxy);
    QObject parent;
    FileInfoModel model(&parent);
    broker.model->d->srcModel = &model;
    EXPECT_NO_FATAL_FAILURE(broker.sortOrder());
    EXPECT_NO_FATAL_FAILURE(broker.setSortRole(1,1));
    EXPECT_NO_FATAL_FAILURE(broker.index(1));
    EXPECT_NO_FATAL_FAILURE(broker.rowCount());
    EXPECT_NO_FATAL_FAILURE(broker.data(QUrl("temp_url"),1));
    EXPECT_NO_FATAL_FAILURE(broker.sort());
    EXPECT_NO_FATAL_FAILURE(broker.refresh(true,1));
    EXPECT_NO_FATAL_FAILURE(broker.fetch(QUrl("temp_url")));
    EXPECT_NO_FATAL_FAILURE(broker.take(QUrl("temp_url")));
}
