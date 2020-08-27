#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include <QEventLoop>
#include <QVariant>


#include "../../../dde-zone/hotzone.h"

using namespace testing;
class HotZoneTest : public Test{
public:
    HotZoneTest():Test(){

    }
    virtual void SetUp() override{
        m_lable = new QLabel();
        hzone = new HotZone(m_lable,false,false);
        hzone1 = new HotZone(m_lable,true,false);
        hzone2 = new HotZone(m_lable,false,true);
        hzone3 = new HotZone(m_lable,true,true);
    }

    virtual void TearDown() override{
        delete hzone;
        delete m_lable;
    }
    HotZone* hzone;
    HotZone* hzone1;
    HotZone* hzone2;
    HotZone* hzone3;
    QLabel* m_lable;
};

TEST_F(HotZoneTest,initSeveralVar)
{
   EXPECT_FALSE(hzone->m_isRight);
   EXPECT_FALSE(hzone->m_isBottom);
   EXPECT_EQ(hzone->corner,HotZone::TopLeft);

   EXPECT_TRUE(hzone1->m_isRight);
   EXPECT_FALSE(hzone1->m_isBottom);
   EXPECT_EQ(hzone1->corner,HotZone::TopRight);


   EXPECT_FALSE(hzone2->m_isRight);
   EXPECT_TRUE(hzone2->m_isBottom);
   EXPECT_EQ(hzone2->corner,HotZone::BottomLeft);


   EXPECT_TRUE(hzone3->m_isRight);
   EXPECT_TRUE(hzone3->m_isBottom);
   EXPECT_EQ(hzone3->corner,HotZone::BottomRight);

}


TEST_F(HotZoneTest,updateEnterPic)
{
    hzone->updateEnterPic();
    QPixmap m_map = hzone->getPixmap(":/images/leftup_corner_hover.svg");
    EXPECT_EQ(hzone->m_pixmap.size(),m_map.size());

    hzone1->updateEnterPic();
    QPixmap m1_map = hzone1->getPixmap(":/images/rightup_corner_hover.svg");
    EXPECT_EQ(hzone1->m_pixmap.size(),m1_map.size());

    hzone2->updateEnterPic();
    QPixmap m2_map = hzone2->getPixmap(":/images/leftdown_corner_hover.svg");
    EXPECT_EQ(hzone2->m_pixmap.size(),m2_map.size());

    hzone3->updateEnterPic();
    QPixmap m3_map = hzone3->getPixmap(":/images/rightdown_corner_hover.svg");
    EXPECT_EQ(hzone3->m_pixmap.size(),m3_map.size());
}

TEST_F(HotZoneTest,initItemAtCorrectCorner)
{
    EXPECT_NE(hzone->m_mainItem,nullptr);
    EXPECT_NE(hzone1->m_mainItem,nullptr);
    EXPECT_NE(hzone2->m_mainItem,nullptr);
    EXPECT_NE(hzone3->m_mainItem,nullptr);

    EXPECT_EQ(m_lable->pos().x(),hzone->m_mainItem->x());
    EXPECT_EQ(m_lable->pos().y(),hzone->m_mainItem->y()-30);

    EXPECT_NE(m_lable->pos().x(),hzone1->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(),hzone1->m_mainItem->y());

    EXPECT_EQ(m_lable->pos().x(),hzone2->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(),hzone2->m_mainItem->y());

    EXPECT_NE(m_lable->pos().x(),hzone3->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(),hzone3->m_mainItem->y());

}
