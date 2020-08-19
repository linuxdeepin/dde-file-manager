#include "models/computermodel.h"

#include <gtest/gtest.h>

namespace {
class TestComputerModel : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestComputerModel";

        model = new ComputerModel;
    }

    void TearDown() override
    {
        std::cout << "end TestComputerModel";
    }

public:
    ComputerModel *model;
};
} // namespace

TEST_F(TestComputerModel, modexIndexIsNotValid)
{
    EXPECT_FALSE(model->index(-1, -1).isValid());
}

TEST_F(TestComputerModel, modelParentIsNotValid)
{
    EXPECT_FALSE(model->parent(model->index(0, 0)).isValid());
}

TEST_F(TestComputerModel, modelRowCountIsZero)
{
    EXPECT_TRUE(model->rowCount() == 0);
}

TEST_F(TestComputerModel, modelHasColumn)
{
    EXPECT_TRUE(model->columnCount() != 0);
}

//TEST_F(TestComputerModel, modelDataIsNull) {
//    EXPECT_STREQ("", model->data(model->index(0, 0)).toString().toStdString().c_str());
//}

//TEST_F(TestComputerModel, modelCanSetData) {
//    EXPECT_TRUE(model->setData(model->index(0, 0), "test"));
//}

TEST_F(TestComputerModel, modelCanNotFindItemByUrl)
{
    EXPECT_FALSE(model->findIndex(DUrl("/")).isValid());
}

TEST_F(TestComputerModel, modelItemCountIsZero)
{
    model->getRootFile();
    EXPECT_EQ(int(0), model->itemCount());
}
