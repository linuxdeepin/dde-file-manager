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
