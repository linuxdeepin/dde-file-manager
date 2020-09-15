#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "../../presenter/gridcore.h"

using namespace testing;

namespace  {
      class GridCoreTest : public Test {
      public:
          GridCoreTest() : Test() {};

          virtual void SetUp() override
          {
              QVector<bool> bk(200, false);
              grid = new GridCore();
              grid->m_cellStatus.insert(1, bk);
              grid->m_cellStatus.insert(2, bk);
              QMap<QString, GPos> map1;
              QMap<GPos, QString> map2;
              map1.insert("string",QPoint(10, 10));
              map2.insert(QPoint(10,10), "string");
              grid->itemGrids.insert(1, map1);
              grid->gridItems.insert(1, map2);
              grid->screensCoordInfo.insert(1, {10, 10});
          }

          virtual void TearDown() override
          {
              delete grid;
          }

          GridCore* grid = nullptr;
      };
}


TEST_F(GridCoreTest,add_item)
{
    int start = grid->overlapItems.size();
    grid->addItem(1, -1, "string");
    int end = grid->overlapItems.size();
    EXPECT_NE(start, end);
    int fitem = grid->itemGrids.value(1).size();
    int fgrid = grid->gridItems.value(1).size();
    grid->addItem(1, 1, "string");
    EXPECT_NE(fgrid, grid->gridItems.value(1).size());
}

TEST_F(GridCoreTest, find_emptyforward)
{
    GIndex gtemp = grid->findEmptyForward(1, 1, 0);
    GIndex gtemp1 = grid->findEmptyForward(1, 4, 3);
    GIndex gtemp2 = grid->findEmptyForward(1, 3, 4);
    EXPECT_EQ(gtemp, 1);
    EXPECT_EQ(gtemp1, 2);
    EXPECT_EQ(gtemp2, 0);
}

TEST_F(GridCoreTest, reloacle_forward)
{
   QStringList slist = grid->reloacleForward(2, 1, 4);
   EXPECT_TRUE(slist.empty());
   QStringList slistf = grid->reloacleForward(1, 110, 110);
   EXPECT_TRUE(slistf.size() == 0);
}

TEST_F(GridCoreTest, count_emptypostion)
{
    QList<GIndex> list1 = grid->emptyPostion(3);
    QList<GIndex> list2 = grid->emptyPostion(1);
    EXPECT_TRUE(list1.empty());
    EXPECT_EQ(200, list2.size());
}

TEST_F(GridCoreTest,to_index_and_pos)
{
    int index = grid->toIndex(1, {10, 10});
    GPos topos = grid->toPos(1,25);
    GPos pos = grid->pos(1,"string");
    EXPECT_EQ(index, 110);
    EXPECT_EQ(topos, QPoint(2,5));
}

TEST_F(GridCoreTest,remove_item)
{
    grid->removeItem(1,"string");
    if(grid->gridItems.contains(1) || grid->itemGrids.contains(1))
    {
        auto tempItemGridsItor = grid->itemGrids.find(1);

        auto pos = tempItemGridsItor.value().take("string");
        auto index = grid->toIndex(1, pos);
        auto cellStatusItor = grid->m_cellStatus.find(1);
        EXPECT_FALSE(cellStatusItor.value()[index]);
    }
}
