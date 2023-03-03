// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/filters/hiddenfilefilter.h"

#include <dfm-framework/dpf.h>

#include "stubext.h"

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
using namespace ddplugin_organizer;

class HiddenFileFilterTest : public testing::Test
{
public:
    virtual void SetUp() override {
        stub.set_lamda((QVariant (EventChannelManager::*)(const QString &, const QString &))&EventChannelManager::push,
                       [this](EventChannelManager *self, const QString &space, const QString &topic) {
            if (topic == "slot_CanvasModel_ShowHiddenFiles" && space == "ddplugin_canvas") {
                getFlag = true;
                return QVariant::fromValue(show);
            }

            return QVariant::fromValue(false);
        });

    }
    virtual void TearDown() override {}
    stub_ext::StubExt stub;
    bool show = true;
    bool getFlag = false;

};

TEST_F(HiddenFileFilterTest, construct)
{
    HiddenFileFilter filter;
    EXPECT_TRUE(filter.show);
    EXPECT_TRUE(getFlag);
}
