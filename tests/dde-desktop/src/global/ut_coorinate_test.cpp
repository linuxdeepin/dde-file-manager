#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define private public
#include "global/coorinate.h"

using namespace testing;
namespace  {
    class CoordinateTest : public Test
    {
    public:
        CoordinateTest():Test()
        {
            p_coor = nullptr;
        }

        virtual void SetUp() override {
            p_coor = new Coordinate(0,0);
        }

        virtual void TearDown() override {
            delete  p_coor;
        }

        Coordinate * p_coor;
    };
}

TEST_F(CoordinateTest, call_constructor)
{
    ASSERT_NE(p_coor, nullptr);
    QPoint pos(0,0);
    Coordinate result(pos);
    EXPECT_EQ(p_coor->position(), result.position());
}

TEST_F(CoordinateTest, get_point)
{
    ASSERT_NE(p_coor, nullptr);
    EXPECT_EQ(p_coor->position().x(), p_coor->d.x);
    EXPECT_EQ(p_coor->position().y(), p_coor->d.y);
}

TEST_F(CoordinateTest, move_to_left)
{
    ASSERT_NE(p_coor, nullptr);
    Coordinate result = p_coor->moveLeft();
    EXPECT_EQ(result.position().x(), p_coor->position().x()-1);

    result = p_coor->moveLeft(5);
    EXPECT_EQ(result.position().x(), p_coor->position().x()-5);
}

TEST_F(CoordinateTest, move_to_right)
{
    ASSERT_NE(p_coor, nullptr);
    Coordinate result = p_coor->moveRight();
    EXPECT_EQ(result.position().x(), p_coor->position().x()+1);

    result = p_coor->moveRight(5);
    EXPECT_EQ(result.position().x(), p_coor->position().x()+5);
}

TEST_F(CoordinateTest, move_to_up)
{
    ASSERT_NE(p_coor, nullptr);
    Coordinate result = p_coor->moveUp();
    EXPECT_EQ(result.position().y(), p_coor->position().y()-1);

    result = p_coor->moveUp(5);
    EXPECT_EQ(result.position().y(), p_coor->position().y()-5);
}

TEST_F(CoordinateTest, move_to_down)
{
    ASSERT_NE(p_coor, nullptr);
    Coordinate result = p_coor->moveDown();
    EXPECT_EQ(result.position().y(), p_coor->position().y()+1);

    result = p_coor->moveDown(5);
    EXPECT_EQ(result.position().y(), p_coor->position().y()+5);
}
