#include "models/dfileselectionmodel.h"

#include <gtest/gtest.h>

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
    }

public:
    DFileSelectionModel *model;
};
} // namespace

TEST_F(TestDFileSelectionModel, isSelected)
{
    EXPECT_FALSE(model->isSelected(QModelIndex()));
}

TEST_F(TestDFileSelectionModel, selectedCount)
{
    EXPECT_EQ(0, model->selectedCount());
}

TEST_F(TestDFileSelectionModel, selectedIndexes)
{
    EXPECT_EQ(0, model->selectedIndexes().count());
}
