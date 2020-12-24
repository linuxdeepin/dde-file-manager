#include <gtest/gtest.h>

#include "interfaces/dfmstyleditemdelegate.h"
#include "interfaces/dfileviewhelper.h"

#include <QAbstractItemView>
#include <QListView>
#include <QObject>
#include <QStyleOptionViewItem>
#include <QStandardItemModel>
#include <QRectF>

//DFM_USE_NAMESPACE

class inhDFileViewHelper;
class inhDFMStyledItemDelegat;
class QListView;

class DFileViewHelperInherit :public DFileViewHelper
{
public:
    explicit DFileViewHelperInherit(QListView *parent):DFileViewHelper(parent){}

    DFileViewHelperInherit() = delete;

    virtual ~DFileViewHelperInherit() override {}

    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const override { return DAbstractFileInfoPointer();}

    virtual DFMStyledItemDelegate *itemDelegate() const override { return nullptr; }

    virtual DFileSystemModel *model() const override { return nullptr; }

    virtual const DUrlList selectedUrls() const override { return DUrlList{}; }

    virtual void select(const QList<DUrl> &list) override {} //nothing to do
};


#include <QPainter>
class DFMStyledItemDelegateInherit :public DFMStyledItemDelegate
{
public:
    DFMStyledItemDelegateInherit(DFileViewHelper *parent):DFMStyledItemDelegate(parent)
    {
        this->initTextLayout(QModelIndex(), new QTextLayout());
        this->initStyleOption(new QStyleOptionViewItem(), QModelIndex());
        this->getCornerGeometryList(QRectF(), QSizeF());
    }

    DFMStyledItemDelegateInherit() = delete;

    virtual ~DFMStyledItemDelegateInherit() override {}

    virtual void updateItemSizeHint() override {}

    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option,
                                        const QModelIndex &index,
                                        bool sizeHintMode = false) const override
    {
        return {};
    }

    //此处封装父类 Delegate 以暴露接口 方便外部调用
    void paintIconMine(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                       QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off)
    {
        DFMStyledItemDelegate::paintIcon(painter,icon,rect,alignment,mode,state);
    }
};

namespace {

class DFMStyledItemDelegateTest : public testing::Test, public QListView
{
public:
    DFMStyledItemDelegate * delegate = nullptr;
    DFileViewHelperInherit* helper = nullptr;
    QStandardItemModel itemModel;

    void SetUp() override
    {
        qDebug() << __PRETTY_FUNCTION__;
        if (!this->model()) {
            this->setModel(&itemModel);
        }

        if (!helper) {
            helper = new DFileViewHelperInherit(this);
        }

        if (!delegate){
            delegate = new DFMStyledItemDelegateInherit(helper);
        }

        //此类必须继承QWidget 否则无法测试 DFMStyledItemDelegate:：paintIcon
        QPainter painter(this);
        ((DFMStyledItemDelegateInherit*)(delegate))->paintIconMine(&painter,QIcon::fromTheme("edit-undo"),QRectF(QPointF(0,0),QPointF(16,16)));

    }

    void TearDown() override
    {
        qDebug() << __PRETTY_FUNCTION__;
    }

};
}

TEST_F(DFMStyledItemDelegateTest, parent)
{
    EXPECT_TRUE(delegate->parent() == helper);
}

TEST_F(DFMStyledItemDelegateTest, editingIndex)
{
    auto indexModel = delegate->editingIndex();
    EXPECT_TRUE(indexModel.row() == -1);
    EXPECT_TRUE(indexModel.column() == -1);
}

TEST_F(DFMStyledItemDelegateTest, editingIndexWidget)
{
    auto pointer = delegate->editingIndexWidget();
    EXPECT_TRUE(pointer == nullptr);
}

TEST_F(DFMStyledItemDelegateTest, sizeHint)
{
    auto size = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    EXPECT_TRUE(size == QSize(-1,-1));
}

TEST_F(DFMStyledItemDelegateTest, destroyEditor)
{
    auto widget = new QWidget ;
    QStyleOptionViewItem styleItem;
    QModelIndex modelIndex;
    delegate->createEditor(widget, styleItem, modelIndex);
    delegate->destroyEditor(widget, modelIndex);
}

TEST_F(DFMStyledItemDelegateTest, hasWidgetIndexs)
{
    EXPECT_TRUE(delegate->hasWidgetIndexs().isEmpty());
}


TEST_F(DFMStyledItemDelegateTest, hideAllIIndexWidget)
{
    //model is empty to call
    auto widget = new QWidget ;
    QStyleOptionViewItem styleItem;
    QModelIndex modelIndex;
    delegate->createEditor(widget, styleItem, modelIndex);

    delegate->hideAllIIndexWidget();

    EXPECT_TRUE(widget->isHidden());

    delegate->destroyEditor(widget, modelIndex);
}

TEST_F(DFMStyledItemDelegateTest,hideNotEditingIndexWidget)
{
    //hideNotEditingIndexWidget method is pass
    delegate->hideNotEditingIndexWidget();
}

TEST_F(DFMStyledItemDelegateTest,commitDataAndCloseActiveEditor)
{
    delegate->commitDataAndCloseActiveEditor();
}

TEST_F(DFMStyledItemDelegateTest,fileNameRect)
{
    //get item filename rect in view, arguments is empty const class;
    EXPECT_TRUE(QRect(QPoint(0,0),QPoint(-1,-1)) == delegate->fileNameRect(QStyleOptionViewItem(),QModelIndex()));
}

TEST_F(DFMStyledItemDelegateTest,paintGeomertys)
{
    //model is empty to call,cause overload the only-virtual method; noting to do
    EXPECT_TRUE(delegate->paintGeomertys(QStyleOptionViewItem(),QModelIndex()).isEmpty());
}

TEST_F(DFMStyledItemDelegateTest,iconSizeLevel)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->iconSizeLevel());
}

TEST_F(DFMStyledItemDelegateTest,minimumIconSizeLevel)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->minimumIconSizeLevel());
}

TEST_F(DFMStyledItemDelegateTest,maximumIconSizeLevel)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->maximumIconSizeLevel());
}

TEST_F(DFMStyledItemDelegateTest,increaseIcon)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->increaseIcon());
}

TEST_F(DFMStyledItemDelegateTest,decreaseIcon)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->decreaseIcon());
}

TEST_F(DFMStyledItemDelegateTest,setIconSizeByIconSizeLevel)
{
    //defualt return -1
    EXPECT_TRUE(-1 == delegate->setIconSizeByIconSizeLevel(0));
}

TEST_F(DFMStyledItemDelegateTest,updateItemSizeHint)
{
    //(noting to do)pass method and overload updateItemSizeHint
    delegate->updateItemSizeHint();
}

#include <QPainter>
#include <QtGlobal>

TEST_F(DFMStyledItemDelegateTest,drawText)
{
    EXPECT_TRUE(1 == delegate->drawText(QModelIndex(),new QPainter(), new QTextLayout(),QRectF(),0.0,QBrush()).size());
}

TEST_F(DFMStyledItemDelegateTest,drawTest2)
{
    EXPECT_TRUE(1 == delegate->drawText(QModelIndex(),new QPainter(),QString("testString"),QRectF(),0.0,QBrush()).size());
}

#include <QColor>
TEST_F(DFMStyledItemDelegateTest,paintCircleList)
{
    delegate->paintCircleList(new QPainter,QRect(),0.0,{QColor()},QColor());
}

#include <QIcon>
#include <QLabel>
#include <testhelper.h>

TEST_F(DFMStyledItemDelegateTest,getIconPixmap)
{
    //program return empty pixmap
    QString longwidthImg = QCoreApplication::applicationDirPath() + "/test_long_width.png";

    //check source image
    if (!QFileInfo::exists(longwidthImg)) {

        QLabel label("long width image");
        label.setFixedSize(1024,1);

        QPixmap loadPixmap = QIcon::fromTheme("edit-undo").pixmap(QSize(16,16));
        QPixmap fitpixmap = loadPixmap.scaled(1024, 40, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        label.setPixmap(fitpixmap);
        label.setStyleSheet("QLabel{background-color:green;}");

        if (!label.pixmap()->isNull()) {
            label.pixmap()->save(longwidthImg);
        }
//        label.show();
//        qApp->exec();
    }

    EXPECT_TRUE(delegate->getIconPixmap(QIcon(),{0,0},0.0).isNull());

    //forever while loop
    delegate->getIconPixmap(QIcon::fromTheme("edit-undo"),{0,0},0.0);

    auto lwPixmap = delegate->getIconPixmap(QIcon(longwidthImg),QSize(1024,40),1.0);

    //get edit-undo pixmap
    QIcon icon = QIcon::fromTheme("edit-undo");
    auto pixmap = delegate->getIconPixmap(icon,QSize(16,16),1.0);
    EXPECT_FALSE(pixmap.isNull());

}


