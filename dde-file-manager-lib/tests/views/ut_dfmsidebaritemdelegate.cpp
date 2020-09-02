#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include <views/dfmsidebarview.h>
#include <interfaces/dfmsidebaritem.h>
#include <views/dfmsidebaritemdelegate.h>

#include<QPainter>

DFM_USE_NAMESPACE
namespace  {
    class DFMSideBarItemDelegateTest:public testing::Test
    {
    public:
        DFMSideBarView m_sidebarView;
        DFMSideBarItemDelegate *m_sideBarItemDelegate{ nullptr };

        virtual void SetUp() override
        {
            m_sidebarView.setItemDelegate(new DFMSideBarItemDelegate(&m_sidebarView));
            m_sideBarItemDelegate = dynamic_cast<DFMSideBarItemDelegate *>(m_sidebarView.itemDelegate());
        }

        virtual void TearDown() override
        {
        }
    };
}


TEST_F(DFMSideBarItemDelegateTest, paintTest){
    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;
    m_sideBarItemDelegate->paint(&painter, option, index);
}

TEST_F(DFMSideBarItemDelegateTest, sizeHintTest){
    QStyleOptionViewItem option;
    QModelIndex index;
    auto size = m_sideBarItemDelegate->sizeHint(option, index);
    EXPECT_NE(0, size.width());
    EXPECT_NE(0, size.height());
}

TEST_F(DFMSideBarItemDelegateTest, sizeHintForTypeTest){
    auto expectValueSeparator = QSize(200, 5) == m_sideBarItemDelegate->sizeHintForType(DFMSideBarItem::Separator);
    EXPECT_TRUE(expectValueSeparator);
    auto expectValueNoSeparator = QSize(0, 0) == m_sideBarItemDelegate->sizeHintForType(DFMSideBarItem::SidebarItem);
    EXPECT_TRUE(expectValueNoSeparator);
}

//TEST_F(DFMSideBarItemDelegateTest, createEditorTest)
//{
//    QStyleOptionViewItem option;
//    QModelIndex index;
//    QWidget *widget = m_sideBarItemDelegate->createEditor(nullptr, option, index);
//    EXPECT_NE(nullptr, widget);
//}


//TEST_F(DFMSideBarItemDelegateTest, setEditorDataTest)
//{
//    QStyleOptionViewItem option;
//    QModelIndex index;
//    QWidget *widget = m_sideBarItemDelegate->createEditor(nullptr, option, index);

//    m_sideBarItemDelegate->setEditorData(widget, index);
//}

//TEST_F(DFMSideBarItemDelegateTest, updateEditorGeometryTest)
//{
//    QStyleOptionViewItem option;
//    QModelIndex index;
//    QWidget *widget = m_sideBarItemDelegate->createEditor(nullptr, option, index);

//    m_sideBarItemDelegate->updateEditorGeometry(widget, option, index);
//}

