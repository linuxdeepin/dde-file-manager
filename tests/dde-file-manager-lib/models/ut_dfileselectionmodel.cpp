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

#include "models/dfileselectionmodel.h"
#include "stub.h"
#include "addr_pri.h"

#include <gtest/gtest.h>

ACCESS_PRIVATE_FIELD(DFileSelectionModel, QItemSelectionModel::SelectionFlags, m_currentCommand);
ACCESS_PRIVATE_FIELD(DFileSelectionModel, QTimer, m_timer);
ACCESS_PRIVATE_FUN(DFileSelectionModel, void(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command), select);
ACCESS_PRIVATE_FUN(DFileSelectionModel, void(), clear);

namespace {
class TestDFileSelectionModel : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFileSelectionModel";

        model = new DFileSelectionModel;
    }

    void TearDown() override
    {
        std::cout << "end TestDFileSelectionModel";
        delete model;
    }

public:
    DFileSelectionModel *model;
};
} // namespace

TEST_F(TestDFileSelectionModel, tstConstructWithParent)
{
    auto m = new DFileSelectionModel(nullptr, nullptr);
    delete m;
}

TEST_F(TestDFileSelectionModel, tstIsSelected)
{
    auto &cmd = access_private_field::DFileSelectionModelm_currentCommand(*model);
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current);
    QModelIndex idx;
    EXPECT_FALSE(model->isSelected(idx));
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                              | QItemSelectionModel::Rows
                                              | QItemSelectionModel::ClearAndSelect);
    EXPECT_FALSE(model->isSelected(idx));
}


TEST_F(TestDFileSelectionModel, tstSelectedCount)
{
    auto &cmd = access_private_field::DFileSelectionModelm_currentCommand(*model);
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                              | QItemSelectionModel::Rows
                                              | QItemSelectionModel::ClearAndSelect);
    EXPECT_EQ(0, model->selectedCount());
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                              | QItemSelectionModel::Rows);
    EXPECT_EQ(0, model->selectedCount());
}

TEST_F(TestDFileSelectionModel, tstSelectedIndexes)
{
    auto &cmd = access_private_field::DFileSelectionModelm_currentCommand(*model);
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                              | QItemSelectionModel::Rows
                                              | QItemSelectionModel::ClearAndSelect);
    EXPECT_EQ(0, model->selectedIndexes().count());
    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                              | QItemSelectionModel::Rows);
    EXPECT_EQ(0, model->selectedIndexes().count());
}

TEST_F(TestDFileSelectionModel, tstSelect)
{
    QItemSelection selection;
    QItemSelectionModel::SelectionFlags cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                                                                  | QItemSelectionModel::Rows);;
    auto &timer = access_private_field::DFileSelectionModelm_timer(*model);
    timer.start();
    call_private_fun::DFileSelectionModelselect(*model, selection, cmd);

    cmd = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current
                                                  | QItemSelectionModel::Rows
                                                  | QItemSelectionModel::ClearAndSelect);
    call_private_fun::DFileSelectionModelselect(*model, selection, cmd);
}

TEST_F(TestDFileSelectionModel, tstClear)
{
    call_private_fun::DFileSelectionModelclear(*model);
}
