#include "views/fileviewhelper.h"
#include "interfaces/dfmviewmanager.h"

#include <DMainWindow>

#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QTextDocument>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "views/dfileview.h"
#include "interfaces/diconitemdelegate.h"
#include "testhelper.h"

namespace {

class TestDIconItemDelegate : public testing::Test
{
public:
    DIconItemDelegate *iconItemDelegate = nullptr;
    DFileView fileview;
    void SetUp() override
    {
        fileview.initDelegate();
        fileview.increaseIcon();
        fileview.decreaseIcon();
        fileview.setIconSizeBySizeIndex(0);
        iconItemDelegate = qobject_cast<DIconItemDelegate *>(fileview.itemDelegate());
    }

    void TearDown() override
    {
        TestHelper::runInLoop([](){}, 10);
    }
};

} // namespace

class DFileViewHelperInh : public DFileViewHelper
{
public:
    explicit DFileViewHelperInh(QAbstractItemView *parent = nullptr) : DFileViewHelper(parent) {}

    virtual ~DFileViewHelperInh() {}

    inline virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const
    {
        Q_UNUSED(index)
        return {};
    }

    inline virtual DFMStyledItemDelegate *itemDelegate() const { return nullptr; }

    inline virtual DFileSystemModel *model() const { return nullptr; }

    inline virtual const DUrlList selectedUrls() const { return {}; }

    inline virtual void select(const QList<DUrl> &list)
    {
        Q_UNUSED(list)
        return;
    }

    virtual void viewFlicker() override {} //nothing to do
};

#ifndef __arm__
TEST_F(TestDIconItemDelegate, test_paint)
{
    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    option.widget = new DMainWindow();
    auto val = option.widget->palette().base().color();
    const QModelIndex &index = iconItemDelegate->expandedIndex();
    iconItemDelegate->paint(painter, option, index);
    delete painter;
    painter = nullptr;

    delete option.widget;
}
#endif

TEST_F(TestDIconItemDelegate, test_sizeHint)
{
    QStyleOptionViewItem viewItem;
    const QModelIndex &index = iconItemDelegate->expandedIndex();
    iconItemDelegate->sizeHint(viewItem, index);
}

TEST_F(TestDIconItemDelegate, test_createEditor)
{
    QWidget w;
    QStyleOptionViewItem viewItem;
    const QModelIndex &index = iconItemDelegate->expandedIndex();
    iconItemDelegate->createEditor(&w, viewItem, index);
}

TEST_F(TestDIconItemDelegate, test_updateEditorGeometry)
{
    QWidget *editor = iconItemDelegate->expandedIndexWidget();
    QStyleOptionViewItem option;
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->updateEditorGeometry(editor, option, index);
}

TEST_F(TestDIconItemDelegate, test_setEditorData)
{
    QWidget *editor = iconItemDelegate->expandedIndexWidget();
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->setEditorData(editor, index);
}

TEST_F(TestDIconItemDelegate, test_paintGeomertys)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->paintGeomertys(option, index);
}

TEST_F(TestDIconItemDelegate, test_paintGeomertys2)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->paintGeomertys(option, index);
}

TEST_F(TestDIconItemDelegate, test_hasWidgetIndexs)
{
    iconItemDelegate->hasWidgetIndexs();
}

TEST_F(TestDIconItemDelegate, test_hideNotEditingIndexWidget)
{
    iconItemDelegate->hideNotEditingIndexWidget();
}

TEST_F(TestDIconItemDelegate, test_expandedIndex)
{
    iconItemDelegate->expandedIndex();
}

TEST_F(TestDIconItemDelegate, test_expandedIndexWidget)
{
    iconItemDelegate->expandedIndexWidget();
}

TEST_F(TestDIconItemDelegate, test_iconSizeLevel)
{
    EXPECT_TRUE(iconItemDelegate->iconSizeLevel() == 0);
}

TEST_F(TestDIconItemDelegate, test_minimumIconSizeLevel)
{
    EXPECT_TRUE(iconItemDelegate->minimumIconSizeLevel() == 0);
}

TEST_F(TestDIconItemDelegate, test_maximumIconSizeLevel)
{
    EXPECT_TRUE(iconItemDelegate->maximumIconSizeLevel() != 0);
}

TEST_F(TestDIconItemDelegate, test_increaseIcon)
{
    EXPECT_TRUE(iconItemDelegate->increaseIcon() != 0);
}

TEST_F(TestDIconItemDelegate, test_decreaseIcon)
{
    EXPECT_TRUE(iconItemDelegate->decreaseIcon() != 0);
}

TEST_F(TestDIconItemDelegate, test_setIconSizeByIconSizeLevel)
{
    EXPECT_TRUE(iconItemDelegate->setIconSizeByIconSizeLevel(1) == 1);
}

TEST_F(TestDIconItemDelegate, test_setIconSizeByIconSizeLevel2)
{
    EXPECT_TRUE(iconItemDelegate->setIconSizeByIconSizeLevel(2) == 2);
}

TEST_F(TestDIconItemDelegate, test_updateItemSizeHint)
{
    iconItemDelegate->updateItemSizeHint();
}

TEST_F(TestDIconItemDelegate, test_FocusTextBackgroundBorderColor)
{
    iconItemDelegate->focusTextBackgroundBorderColor();
}

TEST_F(TestDIconItemDelegate, test_setEnabledTextShadow)
{
    iconItemDelegate->setEnabledTextShadow(true);
    iconItemDelegate->setEnabledTextShadow(false);
}

TEST_F(TestDIconItemDelegate, test_setFocusTextBackgroundBorderColor)
{
    iconItemDelegate->setFocusTextBackgroundBorderColor(QColor::fromRgbF(1, 1, 1, 0.1));
}

TEST_F(TestDIconItemDelegate, test_initTextLayout)
{
    QWidget w;
    QStyleOptionViewItem option;

    option.init(&w);
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->paintGeomertys(option, index);
    QTextLayout layout;
    iconItemDelegate->initTextLayout(index, &layout);
}

TEST_F(TestDIconItemDelegate, test_eventFilter)
{
    QObject object;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    iconItemDelegate->eventFilter(&object, &event);
}

TEST_F(TestDIconItemDelegate, test_drawText)
{
    QModelIndex index = iconItemDelegate->expandedIndex();
    QPainter painter;
    QTextLayout layout;
    QRectF boundingRect(0, 0, 100, 100);
    qreal radius = 1.1;
    const QBrush background(QColor(0xFFFFAE00));
    int flag = 1;
    QColor shadowColor(QColor(0xFFFFAE00));
    QTextOption::WrapMode wordWrap = QTextOption::WrapMode::WordWrap;
    Qt::TextElideMode mode = Qt::TextElideMode::ElideLeft;
    iconItemDelegate->drawText(index, &painter, &layout, boundingRect, radius, background, wordWrap,
                               mode, flag, shadowColor);
}

TEST_F(TestDIconItemDelegate, test_helpEvent)
{
    QHelpEvent event(QEvent::ToolTip, QPoint(10, 10), QPoint(10, 10));
    QAbstractItemView *view = nullptr;
    QStyleOptionViewItem option;
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->helpEvent(&event, view, option, index);
}

TEST_F(TestDIconItemDelegate, test_iconSizeByIconSizeLevel)
{
    iconItemDelegate->iconSizeByIconSizeLevel();
}
