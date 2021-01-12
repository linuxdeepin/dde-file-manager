#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include <QEventLoop>
#include <QVariant>


#include "../dde-zone/hotzone.h"

using namespace testing;

class HotZoneTest : public Test {
public:
    HotZoneTest() : Test() {}

    virtual void SetUp() override
    {
        m_lable = new QLabel();
        m_hzone = new HotZone(m_lable, false, false);
        m_hzone1 = new HotZone(m_lable, true, false);
        m_hzone2 = new HotZone(m_lable, false, true);
        m_hzone3 = new HotZone(m_lable, true, true);
    }

    virtual void TearDown() override
    {
        delete m_hzone;
        delete m_hzone1;
        delete m_hzone2;
        delete m_hzone3;
        delete m_lable;
    }

    HotZone *m_hzone = nullptr;
    HotZone *m_hzone1 = nullptr;
    HotZone *m_hzone2 = nullptr;
    HotZone *m_hzone3 = nullptr;
    QLabel *m_lable = nullptr;
};

TEST_F(HotZoneTest, init_severalvar)
{
   EXPECT_FALSE(m_hzone->m_isRight);
   EXPECT_FALSE(m_hzone->m_isBottom);
   EXPECT_EQ(m_hzone->corner, HotZone::TopLeft);

   EXPECT_TRUE(m_hzone1->m_isRight);
   EXPECT_FALSE(m_hzone1->m_isBottom);
   EXPECT_EQ(m_hzone1->corner, HotZone::TopRight);


   EXPECT_FALSE(m_hzone2->m_isRight);
   EXPECT_TRUE(m_hzone2->m_isBottom);
   EXPECT_EQ(m_hzone2->corner, HotZone::BottomLeft);


   EXPECT_TRUE(m_hzone3->m_isRight);
   EXPECT_TRUE(m_hzone3->m_isBottom);
   EXPECT_EQ(m_hzone3->corner, HotZone::BottomRight);
}

TEST_F(HotZoneTest, update_enterpic)
{
    m_hzone->updateEnterPic();
    QPixmap pixmap = m_hzone->getPixmap(":/images/leftup_corner_hover.svg");
    EXPECT_EQ(m_hzone->m_pixmap.size(), pixmap.size());

    m_hzone1->updateEnterPic();
    QPixmap pixmap1 = m_hzone1->getPixmap(":/images/rightup_corner_hover.svg");
    EXPECT_EQ(m_hzone1->m_pixmap.size(), pixmap1.size());

    m_hzone2->updateEnterPic();
    QPixmap pixmap2 = m_hzone2->getPixmap(":/images/leftdown_corner_hover.svg");
    EXPECT_EQ(m_hzone2->m_pixmap.size(), pixmap2.size());

    m_hzone3->updateEnterPic();
    QPixmap pixmap3 = m_hzone3->getPixmap(":/images/rightdown_corner_hover.svg");
    EXPECT_EQ(m_hzone3->m_pixmap.size(), pixmap3.size());
}

TEST_F(HotZoneTest, init_item_atcorrectcorner)
{
    EXPECT_NE(m_hzone->m_mainItem, nullptr);
    EXPECT_NE(m_hzone1->m_mainItem, nullptr);
    EXPECT_NE(m_hzone2->m_mainItem, nullptr);
    EXPECT_NE(m_hzone3->m_mainItem, nullptr);

    EXPECT_EQ(m_lable->pos().x(), m_hzone->m_mainItem->x());
    EXPECT_EQ(m_lable->pos().y(), m_hzone->m_mainItem->y()-30);

    EXPECT_NE(m_lable->pos().x(), m_hzone1->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(), m_hzone1->m_mainItem->y());

    EXPECT_EQ(m_lable->pos().x(), m_hzone2->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(), m_hzone2->m_mainItem->y());

    EXPECT_NE(m_lable->pos().x(), m_hzone3->m_mainItem->x());
    EXPECT_NE(m_lable->pos().y(), m_hzone3->m_mainItem->y());
}
