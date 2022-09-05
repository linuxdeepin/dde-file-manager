// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusinterface/dbustype.h"

#include <QDebug>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestDiskInfo: public testing::Test {
public:
    DiskInfo *m_info {};

    void SetUp() override
    {
        DiskInfo::registerMetaType();
        m_info = new DiskInfo;
        m_info->ID = "123";
    }

    void TearDown() override
    {
        delete m_info;
    }
};
}

TEST_F(TestDiskInfo, debug)
{
    qDebug() << *m_info;
}

// 据源码所述, QDBusArgument数据只能单一流向, 并且QDBusArgument显示共享, 所以这个用例永远会失败, 暂时屏蔽, 后面再来重写逻辑
/*TEST_F(TestDiskInfo, stream)
{
    QDBusArgument arg;
    DiskInfo info_a;
    info_a.ID = "111";
    arg << info_a;
    DiskInfo info_b;
    arg >> info_b;
    qDebug() <<  "===>" << info_a.ID.toStdString().c_str() << info_b.ID.toStdString().c_str();
    EXPECT_STREQ(info_a.ID.toStdString().c_str(), info_b.ID.toStdString().c_str());
}*/
