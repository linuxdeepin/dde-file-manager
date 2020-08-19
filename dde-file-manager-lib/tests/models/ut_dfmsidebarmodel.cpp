#include "models/dfmsidebarmodel.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

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
