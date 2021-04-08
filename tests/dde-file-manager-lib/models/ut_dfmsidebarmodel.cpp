/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "models/dfmsidebarmodel.h"
#include "stub.h"
#include "addr_pri.h"

#include <gtest/gtest.h>
#include <QMimeData>
#include <QTimer>

DFM_USE_NAMESPACE

ACCESS_PRIVATE_FUN(DFMSideBarModel, QByteArray(const QModelIndexList &) const, generateMimeData);
ACCESS_PRIVATE_FUN(DFMSideBarModel, int(const QByteArray &) const, getRowIndexFromMimeData);

namespace {
class TestDFMSideBarModel : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMSideBarModel";
        model = new DFMSideBarModel;
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSideBarModel";
        QEventLoop loop;
        QTimer::singleShot(200, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete model;
    }

public:
    DFMSideBarModel *model;
};
} // namespace

TEST_F(TestDFMSideBarModel, noItemInThatIndex)
{
    EXPECT_EQ(nullptr, model->itemFromIndex(0));
    EXPECT_EQ(nullptr, model->itemFromIndex(QModelIndex()));
}

TEST_F(TestDFMSideBarModel, noIndexInNullItem)
{
    EXPECT_FALSE(model->indexFromItem(nullptr).isValid());
}

TEST_F(TestDFMSideBarModel, tstGenerateMimeData)
{
    auto idx = model->index(0, 0);
    QModelIndexList idxes;
    idxes << idx;
    call_private_fun::DFMSideBarModelgenerateMimeData(*model, idxes);
}

TEST_F(TestDFMSideBarModel, tstGetRowIndexFromMimeData)
{
    QByteArray ba("1");
    auto result = call_private_fun::DFMSideBarModelgetRowIndexFromMimeData(*model, ba);
    EXPECT_FALSE(1 == result);
}

TEST_F(TestDFMSideBarModel, tstMimeData)
{
    QModelIndexList lst;
    auto p = model->mimeData(lst);
    EXPECT_TRUE(nullptr == p);
}

TEST_F(TestDFMSideBarModel, tstCanDropMimeData)
{
    auto *data = new QMimeData();
    data->setData("application/x-dfmsidebaritemmodeldata", "Test");
    auto act = Qt::DropAction::MoveAction;
    QModelIndex parent;
    EXPECT_FALSE(model->canDropMimeData(nullptr, act, 0, 0, parent));
    EXPECT_FALSE(model->canDropMimeData(data, act, -1, 0, parent));
    EXPECT_FALSE(model->canDropMimeData(data, act, 0, 0, parent));
    delete data;
}
