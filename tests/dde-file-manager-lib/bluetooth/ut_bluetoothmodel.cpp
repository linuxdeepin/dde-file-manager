// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    EXPECT_TRUE(m_model->adapterById("123")->id().contains("123"));
}

TEST_F(TestBluetoothModel, removeDevice)
{
    auto adapter_size = m_model->adapters().size();
    BluetoothAdapter adapter;
    adapter.setId("123");
    m_model->addAdapter(&adapter);
    EXPECT_EQ(m_model->adapters().size(), adapter_size + 1);

    m_model->removeAdapater("123");
    EXPECT_EQ(m_model->adapters().size(), adapter_size);
}


