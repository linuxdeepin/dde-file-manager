#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "presenter/gridcore.h"

using namespace testing;

namespace  {
      class GridCoreTest : public Test {
      public:
          GridCoreTest() : Test() {};

          virtual void SetUp() override
          {
              QVector<bool> bk(200, false);
              m_grid = new GridCore();
              m_grid->m_cellStatus.insert(1, bk);
              m_grid->m_cellStatus.insert(2, bk);
              QMap<QString, GPos> map1;
              QMap<GPos, QString> map2;
              map1.insert("string",QPoint(10, 10));
              map2.insert(QPoint(10,10), "string");
              m_grid->itemGrids.insert(1, map1);
              m_grid->gridItems.insert(1, map2);
              m_grid->screensCoordInfo.insert(1, {10, 10});
          }

          virtual void TearDown() override
          {
              delete m_grid;
          }

          GridCore* m_grid = nullptr;
      };
}


TEST_F(GridCoreTest, test_additem)
{
    int start = m_grid->overlapItems.size();
    m_grid->addItem(1, -1, "string");
    int end = m_grid->overlapItems.size();
    EXPECT_NE(start, end);

    int fitem = m_grid->itemGrids.value(1).size();
    int fgrid = m_grid->gridItems.value(1).size();
    m_grid->addItem(1, 1, "string");
    EXPECT_NE(fgrid, m_grid->gridItems.value(1).size());

    m_grid->m_cellStatus[1] = QVector<bool>{};
    m_grid->addItem(1, 1, "test01");
    m_grid->gridItems.remove(1);
    m_grid->m_cellStatus[1] = QVector<bool>{0, 0, 0};
    m_grid->addItem(1, 1, "test01");
    EXPECT_EQ(m_grid->m_cellStatus.value(1)[1], true);

}

TEST_F(GridCoreTest, test_findemptyforward)
{
    GIndex gtemp = m_grid->findEmptyForward(1, 1, 0);
    GIndex gtemp1 = m_grid->findEmptyForward(1, 4, 3);
    GIndex gtemp2 = m_grid->findEmptyForward(1, 3, 4);
    EXPECT_EQ(gtemp, 1);
    EXPECT_EQ(gtemp1, 2);
    EXPECT_EQ(gtemp2, 0);

    m_grid->m_cellStatus.remove(1);
    GIndex index = m_grid->findEmptyForward(1, 1, 1);
    EXPECT_EQ(index, 1);

    m_grid->m_cellStatus.insert(1, QVector<bool>{1,1,1});
    GIndex index1 = m_grid->findEmptyForward(1, 1, 1);
    EXPECT_EQ(0, index1);
}

TEST_F(GridCoreTest, test_reloacleforward)
{
   QStringList slist = m_grid->reloacleForward(2, 1, 4);
   EXPECT_TRUE(slist.empty());

   QStringList slistf = m_grid->reloacleForward(1, 110, 110);
   EXPECT_TRUE(slistf.size() == 0);

   auto position = m_grid->toPos(1, 1);
   QMap<QPoint, QString> map;
   QString test("test01");
   map.insert(position, test);
   m_grid->gridItems.insert(1, map);
   QStringList strlist = m_grid->reloacleForward(1, 1, 1);

   EXPECT_EQ(strlist[0], test);
}

TEST_F(GridCoreTest, test_countemptypostion)
{
    QList<GIndex> list1 = m_grid->emptyPostion(3);
    QList<GIndex> list2 = m_grid->emptyPostion(1);

    EXPECT_TRUE(list1.empty());
    EXPECT_EQ(200, list2.size());
}

TEST_F(GridCoreTest,test_toindexnandpos)
{
    int index = m_grid->toIndex(1, {10, 10});
    GPos topos = m_grid->toPos(1,25);
    GPos pos = m_grid->pos(1,"string");
    EXPECT_EQ(index, 110);
    EXPECT_EQ(topos, QPoint(2,5));

    m_grid->screensCoordInfo.insert(1, QPair<int,int>(10, 10));
    const GPos mypos = m_grid->pos(1,"test01");
    EXPECT_EQ(mypos.x(), 9);
    EXPECT_EQ(mypos.y(), 9);


}

TEST_F(GridCoreTest,test_removeitem)
{
    m_grid->removeItem(1,"string");
    if(m_grid->gridItems.contains(1) || m_grid->itemGrids.contains(1))
    {
        auto tempItemGridsItor = m_grid->itemGrids.find(1);

        auto pos = tempItemGridsItor.value().take("string");
        auto index = m_grid->toIndex(1, pos);
        auto cellStatusItor = m_grid->m_cellStatus.find(1);
        EXPECT_FALSE(cellStatusItor.value()[index]);
    }
    m_grid->gridItems.remove(1);
    m_grid->itemGrids.remove(1);
    m_grid->removeItem(1, "test");//覆盖打印信息

    QMap<QPoint, QString> map;
    QMap<QString, QPoint> map1;
    map.insert(QPoint(1, 1),"string");
    map1.insert("string", QPoint(1, 1));
    m_grid->gridItems.insert(1, map);
    m_grid->itemGrids.insert(1, map1);
    m_grid->m_cellStatus.remove(1);
    m_grid->removeItem(1,"string");//覆盖打印信息
}

TEST_F(GridCoreTest, test_reloacle)
{
    QStringList strlist;
    strlist = m_grid->reloacle(1, 1, 1, 1);
    QStringList compare;

    auto end = m_grid->findEmptyBackward(1, 1, 1);
    compare << m_grid->reloacleBackward(1, 1, end);
    auto start = m_grid->findEmptyForward(1, 0, 1);
    compare << m_grid->reloacleForward(1, start, 0);

    EXPECT_EQ(strlist, compare);
}

TEST_F(GridCoreTest, test_findemptybackward)
{
    m_grid->m_cellStatus.remove(1);
    GIndex index = m_grid->findEmptyBackward(1, 1, 1);
    EXPECT_EQ(1, index);

    m_grid->m_cellStatus.insert(1, QVector<bool>{1, 0});
    GIndex index1 = m_grid->findEmptyBackward(1, 1, 0);
    EXPECT_EQ(1, index1);

    GIndex index2 = m_grid->findEmptyBackward(1, 1, 1);
    EXPECT_EQ(1, index2);

    m_grid->m_cellStatus[1] = QVector<bool>{1, 1, 1};
    GIndex index3 = m_grid->findEmptyBackward(1, 1, 1);
    EXPECT_EQ(2, index3);
}

TEST_F(GridCoreTest, test_reloacleBackward)
{
    m_grid->gridItems.remove(1);
    QStringList strlist = m_grid->reloacleBackward(1, 1, 1);

    EXPECT_EQ(QStringList(), strlist);

    auto position = m_grid->toPos(1, 1);
    QMap<QPoint, QString> map;
    QString test("test01");

    map[position] = test;
    m_grid->gridItems.insert(1, map);
    QStringList strlist1 = m_grid->reloacleBackward(1, 1, 1);

    EXPECT_EQ(test, strlist1[0]);
}


