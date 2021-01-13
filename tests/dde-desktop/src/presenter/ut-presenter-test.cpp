#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "presenter/apppresenter.h"
#include "presenter/gridmanager.h"

#include <QEventLoop>
#include <QVariant>

using namespace testing;
namespace  {
class PresenterTest : public Test
{
public:
    PresenterTest() :
            Test()
          ,ins(Presenter::instance())
    {}
    Presenter *ins;
    QStringList m_group;
    QStringList m_key;
    QVariantList m_value;
    QObject obj;
    void init(QEventLoop *loop)
    {
        ins->init();
        QObject::connect(ins,&Presenter::setConfig,&obj,[=](const QString &group, const QString &key, const QVariant &value){
            m_group.append(group);
            m_key.append(key);
            m_value.append(value);
            loop->exit();
        },Qt::QueuedConnection);
    }
};
}

TEST_F(PresenterTest, sort_role_changed)
{
    QEventLoop loop;
    init(&loop);
    ins->onSortRoleChanged(1,Qt::AscendingOrder);
    loop.exec();
    EXPECT_EQ(QString("GeneralConfig"),m_group[0]);
    EXPECT_EQ(QString("SortBy"),m_key[0]);
    EXPECT_EQ(1,m_value[0].toInt());

    EXPECT_EQ(QString("GeneralConfig"),m_group[1]);
    EXPECT_EQ(QString("SortOrder"),m_key[1]);
    EXPECT_EQ(Qt::AscendingOrder,static_cast<Qt::SortOrder>(m_value[1].toInt()));
}

TEST_F(PresenterTest, auto_align_toggled)
{
    bool org = GridManager::instance()->autoArrange();
    QEventLoop loop;
    init(&loop);
    ins->onAutoAlignToggled();
    loop.exec();
    EXPECT_EQ(QString("GeneralConfig"),m_group[0]);
    EXPECT_EQ(QString("AutoSort"),m_key[0]);
    EXPECT_NE(org,m_value[0].toBool());

    ins->onAutoAlignToggled();
    loop.exec();
    EXPECT_EQ(QString("GeneralConfig"),m_group[1]);
    EXPECT_EQ(QString("AutoSort"),m_key[1]);
    EXPECT_EQ(org,m_value[1].toBool());
}

TEST_F(PresenterTest, auto_merge_toggled)
{
    bool org = GridManager::instance()->autoMerge();
    QEventLoop loop;
    init(&loop);
    ins->onAutoMergeToggled();
    loop.exec();
    EXPECT_EQ(QString("GeneralConfig"),m_group[0]);
    EXPECT_EQ(QString("AutoMerge"),m_key[0]);
    EXPECT_EQ(org,m_value[0].toBool());

//    ins->onAutoMergeToggled();
//    loop.exec();
//    EXPECT_EQ(QString("GeneralConfig"),m_group[1]);
//    EXPECT_EQ(QString("AutoMerge"),m_key[1]);
//    EXPECT_EQ(org,m_value[1].toBool());
}

TEST_F(PresenterTest, icon_level_change)
{
    QEventLoop loop;
    init(&loop);
    for (int i = 0; i < 3; ++i){
        ins->OnIconLevelChanged(i);
        loop.exec();
        EXPECT_EQ(QString("GeneralConfig"),m_group[i]);
        EXPECT_EQ(QString("IconLevel"),m_key[i]);
        EXPECT_EQ(i,m_value[i].toInt());
    }
}
