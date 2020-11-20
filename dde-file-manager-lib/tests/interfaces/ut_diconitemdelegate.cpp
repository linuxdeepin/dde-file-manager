#include "views/fileviewhelper.h"
#include "interfaces/dfmviewmanager.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QIcon>
#include <QPainter>
#include <QTextDocument>
#include <QEvent>

#define protected public
#define private public
#include "interfaces/diconitemdelegate.h"
#include "views/dfileview.h"

namespace  {
class TestDIconItemDelegate : public testing::Test
{
public:
    DIconItemDelegate *DIconIdl;
    void SetUp() override
    {
        DFileView *dfileview = new DFileView();
        dfileview->initDelegate();
        dfileview->increaseIcon();
        dfileview->decreaseIcon();
        dfileview ->setIconSizeBySizeIndex(0);

        DIconIdl = qobject_cast<DIconItemDelegate *>(dfileview->itemDelegate());

        std::cout << "start TestDIconItemDelegate";
    }
    void TearDown() override
    {
        delete  DIconIdl;
        DIconIdl = nullptr;
        std::cout << "end TestDIconItemDelegate";
    }
};
}
TEST_F(TestDIconItemDelegate, test_paint)
{
    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    const QModelIndex &index =  DIconIdl->expandedIndex();
    DIconIdl->paint(painter, option, index);
    delete painter;
    painter = nullptr;
}

TEST_F(TestDIconItemDelegate, test_sizeHint)
{
    QStyleOptionViewItem viewItem;
    const QModelIndex &index =  DIconIdl->expandedIndex();
    DIconIdl->sizeHint(viewItem, index);
}

TEST_F(TestDIconItemDelegate, test_createEditor)
{
    QWidget w;
    QStyleOptionViewItem viewItem;
    const QModelIndex &index =  DIconIdl->expandedIndex();
    DIconIdl->createEditor(&w, viewItem, index);
}

TEST_F(TestDIconItemDelegate, test_updateEditorGeometry)
{
    QWidget *editor = DIconIdl->expandedIndexWidget();
    QStyleOptionViewItem option;
    QModelIndex index = DIconIdl->expandedIndex();
    DIconIdl->updateEditorGeometry(editor, option, index);
}

TEST_F(TestDIconItemDelegate, test_setEditorData)
{
    QWidget *editor = DIconIdl->expandedIndexWidget();
    QModelIndex index =  DIconIdl->expandedIndex();
    DIconIdl->setEditorData(editor,  index);
}

TEST_F(TestDIconItemDelegate, test_paintGeomertys)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index =  DIconIdl->expandedIndex();
    DIconIdl->paintGeomertys(option, index);
}

TEST_F(TestDIconItemDelegate, test_paintGeomertys2)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index = DIconIdl->expandedIndex();
    DIconIdl->paintGeomertys(option, index);
}

TEST_F(TestDIconItemDelegate, test_hasWidgetIndexs)
{
    DIconIdl->hasWidgetIndexs();
}

TEST_F(TestDIconItemDelegate, test_hideNotEditingIndexWidget)
{
    DIconIdl->hideNotEditingIndexWidget();
}

TEST_F(TestDIconItemDelegate, test_expandedIndex)
{
    DIconIdl->expandedIndex();
}

TEST_F(TestDIconItemDelegate, test_expandedIndexWidget)
{
    DIconIdl->expandedIndexWidget();
}

TEST_F(TestDIconItemDelegate, test_iconSizeLevel)
{
    EXPECT_TRUE(DIconIdl->iconSizeLevel() == 0);
}

TEST_F(TestDIconItemDelegate, test_minimumIconSizeLevel)
{
    EXPECT_TRUE(DIconIdl->minimumIconSizeLevel() == 0);
}

TEST_F(TestDIconItemDelegate, test_maximumIconSizeLevel)
{
    EXPECT_TRUE(DIconIdl->maximumIconSizeLevel() != 0);
}

TEST_F(TestDIconItemDelegate, test_increaseIcon)
{
    EXPECT_TRUE(DIconIdl->increaseIcon() != 0);
}

TEST_F(TestDIconItemDelegate, test_decreaseIcon)
{
    EXPECT_TRUE(DIconIdl->decreaseIcon() != 0);
}

TEST_F(TestDIconItemDelegate, test_setIconSizeByIconSizeLevel)
{
    EXPECT_TRUE(DIconIdl->setIconSizeByIconSizeLevel(1) == 1);
}

TEST_F(TestDIconItemDelegate, test_setIconSizeByIconSizeLevel2)
{
    EXPECT_TRUE(DIconIdl->setIconSizeByIconSizeLevel(2) == 2);
}

TEST_F(TestDIconItemDelegate, test_updateItemSizeHint)
{
    DIconIdl->updateItemSizeHint();
}

TEST_F(TestDIconItemDelegate, test_FocusTextBackgroundBorderColor)
{
    DIconIdl->focusTextBackgroundBorderColor();
}

TEST_F(TestDIconItemDelegate, test_setEnabledTextShadow)
{
    DIconIdl->setEnabledTextShadow(true);
    DIconIdl->setEnabledTextShadow(false);
}

TEST_F(TestDIconItemDelegate, test_setFocusTextBackgroundBorderColor)
{
    DIconIdl->setFocusTextBackgroundBorderColor(QColor::fromRgbF(1, 1, 1, 0.1));
}

TEST_F(TestDIconItemDelegate, test_initTextLayout)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index = DIconIdl->expandedIndex();
    DIconIdl->paintGeomertys(option, index);
    QTextLayout layout;
    DIconIdl->initTextLayout(index, &layout);
}

TEST_F(TestDIconItemDelegate, test_eventFilter)
{
    QObject *object = new QObject();
    QEvent *event = new QEvent(QEvent::KeyPress);
    DIconIdl->eventFilter(object, event);
}


TEST_F(TestDIconItemDelegate, test_drawText)
{
    QModelIndex index = DIconIdl->expandedIndex();
    QPainter *painter = new QPainter();
    QTextLayout *layout = new QTextLayout();
    QRectF boundingRect(0, 0, 100, 100);
    qreal radius = 1.1;
    const QBrush background(QColor(0xFFFFAE00));
    int flag = 1;
    QColor shadowColor(QColor(0xFFFFAE00));
    QTextOption::WrapMode wordWrap = QTextOption::WrapMode::WordWrap;
    Qt::TextElideMode mode = Qt::TextElideMode::ElideLeft;
    DIconIdl->drawText(index, painter, layout, boundingRect, radius, background, wordWrap, mode, flag, shadowColor)  ;
}

TEST_F(TestDIconItemDelegate, test_helpEvent)
{
    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip, QPoint(10, 10), QPoint(10, 10));
    QAbstractItemView *view = nullptr;
    QStyleOptionViewItem option;
    QModelIndex index =  DIconIdl->expandedIndex();
    DIconIdl->helpEvent(event, view, option, index);
}

TEST_F(TestDIconItemDelegate, test_iconSizeByIconSizeLevel)
{
    DIconIdl->iconSizeByIconSizeLevel();
}
