// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "broker/fileinfomodelbroker.h"
#include "model/fileinfomodel.h"

#include <dfm-framework/dpf.h>
#include "stubext.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

TEST(FileInfoModelBroker,test)
{
    FileInfoModel model ;
    FileInfoModelBroker broker(&model);
    QModelIndex index(1,1,nullptr,nullptr);

    EXPECT_NO_FATAL_FAILURE(broker.files());
    EXPECT_NO_FATAL_FAILURE(broker.refresh(index));
    EXPECT_NO_FATAL_FAILURE(broker.modelState());
    EXPECT_NO_FATAL_FAILURE(broker.updateFile(QUrl("temp_url")));
    EXPECT_NO_FATAL_FAILURE(broker.onDataReplaced(QUrl("temp_url_old"),QUrl("temp_url_new")));
}
