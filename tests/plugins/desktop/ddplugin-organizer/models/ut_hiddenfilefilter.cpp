/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
