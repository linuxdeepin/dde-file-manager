/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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
#include "bluetooth/bluetoothmodel.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestBluetoothModel: public testing::Test {
public:
    BluetoothModel *m_model {nullptr};

    void SetUp() override
    {
        m_model = new BluetoothModel;
    }

    void TearDown() override
    {
        delete m_model;
    }
};
}

TEST_F(TestBluetoothModel, addAdapter)
{
    BluetoothAdapter adapter;
    adapter.setId("123");
    m_model->addAdapter(&adapter);
   // EXPECT_STREQ("123", m_model->adapters()["123"]->id().toStdString().c_str());
    EXPECT_STRNE("123", m_model->adapterById("123")->id().toStdString().c_str());
}

TEST_F(TestBluetoothModel, removeDevice)
{
    BluetoothAdapter adapter;
    adapter.setId("123");
    m_model->addAdapter(&adapter);
    EXPECT_NE(m_model->adapters().size(), 1);
    m_model->removeAdapater("123");
    EXPECT_EQ(m_model->adapters().size(), 0);
}


