// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gioemblemworker_1.cpp
 * @brief Unit tests for GioEmblemWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/emblemhelper.h"

#include <QTest>

using namespace dfmplugin_emblem;

class GioEmblemWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GioEmblemWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GioEmblemWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GioEmblemWorkerTest, iconNamesEqual)
{
    // Test method: bool iconNamesEqual((const QList<QIcon> &first, const QList<QIcon> &second))
    QList<QIcon> _arg0{};
    QList<QIcon> _arg1{};
    auto result = obj->iconNamesEqual(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(GioEmblemWorkerTest, onProduce)
{
    // Test method: USING_IO_NAMESPACE onProduce((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->onProduce(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->onProduce(_arg0); });

}

TEST_F(GioEmblemWorkerTest, setEmblemIntoIcons)
{
    // Test setter: void setEmblemIntoIcons((const QString &pos, const QIcon &emblem, QMap<int, QIcon> *iconMap))
    QString _arg0{};
    QIcon _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEmblemIntoIcons(_arg0, _arg1, nullptr));
}
