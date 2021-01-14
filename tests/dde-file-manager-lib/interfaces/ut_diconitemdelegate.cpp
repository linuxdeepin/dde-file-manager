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
#include "interfaces/diconitemdelegate.cpp"

namespace {

class TestDIconItemDelegate : public testing::Test
{
public:
    DIconItemDelegate *iconItemDelegate;
    DFileView *fileview;
    void SetUp() override
    {
        fileview = new DFileView();
        fileview->initDelegate();
        fileview->increaseIcon();
        fileview->decreaseIcon();
        fileview->setIconSizeBySizeIndex(0);

        iconItemDelegate = qobject_cast<DIconItemDelegate *>(fileview->itemDelegate());
    }

    void TearDown() override
    {
        delete iconItemDelegate;
        iconItemDelegate = nullptr;
        delete fileview;
        fileview = nullptr;
    }
};

} // namespace

TEST_F(TestDIconItemDelegate, test_boundingrect)
{
    QList<QRectF> rects;
    rects.append(QRectF { QPointF { 0, 0 }, QPointF { 100, 100 } });
    rects.append(QRectF { QPointF { 100, 100 }, QPointF { 200, 200 } });

    boundingRect(rects);

    rects.clear();

    rects.append(QRectF { QPointF { 100, 100 }, QPointF { 200, 200 } });
    rects.append(QRectF { QPointF { 0, 0 }, QPointF { 300, 300 } });

    boundingRect({});
    boundingRect(rects);

    rects.clear();
}

TEST_F(TestDIconItemDelegate, test_boundPath)
{
    QList<QRectF> rects;
    rects.append(QRectF { QPointF { 0, 0 }, QPointF { 100, 100 } });
    rects.append(QRectF { QPointF { 100, 100 }, QPointF { 200, 200 } });

    boundingPath(rects, 1.0, 1);

    rects.clear();

    rects.append(QRectF { QPointF { 100, 100 }, QPointF { 200, 200 } });
    rects.append(QRectF { QPointF { 0, 0 }, QPointF { 300, 300 } });

    boundingPath(rects, 1.0, 1);
}

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
};

TEST_F(TestDIconItemDelegate, test_textFormat)
{
    TagTextFormat noObjFormat(0, { QColor("red"), QColor("blue") }, QColor("green"));

    noObjFormat.colors();
    noObjFormat.borderColor();
    noObjFormat.diameter();

    TagTextFormat imgFormat(1, { QColor("red"), QColor("blue") }, QColor("green"));
    imgFormat.colors();
    imgFormat.borderColor();
    imgFormat.diameter();

    FileTagObjectInterface interface;
    interface.intrinsicSize(nullptr, 0, noObjFormat);

    QPainter *painter = new QPainter;
    interface.drawObject(painter, { 0, 0, 0, 0 }, nullptr, 0, imgFormat);

    QListView fileView;
    DFileViewHelperInh *helper = new DFileViewHelperInh(&fileView);
    DFileSystemModel *model = new DFileSystemModel(helper);
    fileView.setModel(model);
    DIconItemDelegate *delegate = new DIconItemDelegate(helper);
    ExpandedItem *item = new ExpandedItem(delegate);
    DIconItemDelegatePrivate *delegatePrivate = new DIconItemDelegatePrivate(delegate);

    item->opacity();

    item->setOpacity(0.0);
    item->setOpacity(3.0);

    auto event = new QPaintEvent(QRect(0, 0, 0, 0));

    item->paintEvent(event);

    item->option.text = "testString";

    item->iconPixmap = QIcon::fromTheme("edit-undo").pixmap(QSize(16, 16));

    item->sizeHint();

    item->heightForWidth(10);

    item->heightForWidth(item->width());

    item->paintEvent(event);

    item->textGeometry();

    qApp->processEvents();

    delegatePrivate->textSize("testString", QFontMetrics(QFont()), 20);

    QModelIndex modelIndex;
    delegatePrivate->getFileIconPixmap(modelIndex,
                                       QIcon::fromTheme("edit-undo").pixmap(QSize(16, 16)),
                                       QSize(16, 16), QIcon::Mode::Normal, 0.0);
}

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
    QObject *object = new QObject();
    QEvent *event = new QEvent(QEvent::KeyPress);
    iconItemDelegate->eventFilter(object, event);
}

TEST_F(TestDIconItemDelegate, test_drawText)
{
    QModelIndex index = iconItemDelegate->expandedIndex();
    QPainter *painter = new QPainter();
    QTextLayout *layout = new QTextLayout();
    QRectF boundingRect(0, 0, 100, 100);
    qreal radius = 1.1;
    const QBrush background(QColor(0xFFFFAE00));
    int flag = 1;
    QColor shadowColor(QColor(0xFFFFAE00));
    QTextOption::WrapMode wordWrap = QTextOption::WrapMode::WordWrap;
    Qt::TextElideMode mode = Qt::TextElideMode::ElideLeft;
    iconItemDelegate->drawText(index, painter, layout, boundingRect, radius, background, wordWrap,
                               mode, flag, shadowColor);
}

TEST_F(TestDIconItemDelegate, test_helpEvent)
{
    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip, QPoint(10, 10), QPoint(10, 10));
    QAbstractItemView *view = nullptr;
    QStyleOptionViewItem option;
    QModelIndex index = iconItemDelegate->expandedIndex();
    iconItemDelegate->helpEvent(event, view, option, index);
}

TEST_F(TestDIconItemDelegate, test_iconSizeByIconSizeLevel)
{
    iconItemDelegate->iconSizeByIconSizeLevel();
}
