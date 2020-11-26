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

class inhDFileViewHelper :public DFileViewHelper
{
public:
    explicit inhDFileViewHelper(QListView *parent):DFileViewHelper(parent){}

    inhDFileViewHelper() = delete;

    virtual ~inhDFileViewHelper() override {}

    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const override { return DAbstractFileInfoPointer();}

    virtual DFMStyledItemDelegate *itemDelegate() const override { return nullptr; }

    virtual DFileSystemModel *model() const override { return nullptr; }

    virtual const DUrlList selectedUrls() const override { return DUrlList{}; }

    virtual void select(const QList<DUrl> &list) override {} //nothing to do
};



class inhDFMStyledItemDelegat :public DFMStyledItemDelegate
{
public:
    inhDFMStyledItemDelegat(DFileViewHelper *parent):DFMStyledItemDelegate(parent)
    {
        this->initTextLayout(QModelIndex(),new QTextLayout());
        this->initStyleOption(new QStyleOptionViewItem(), QModelIndex());
        this->getCornerGeometryList(QRectF(),QSizeF());
        //        this->paintIcon(new QPainter(),QIcon(),QRectF());
    }

    inhDFMStyledItemDelegat() = delete;

    virtual ~inhDFMStyledItemDelegat() override {}

    virtual void updateItemSizeHint() override {}

    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option,
                                        const QModelIndex &index,
                                        bool sizeHintMode = false) const override
    {
        return {};
    }

};


namespace {

class DFMStyledItemDelegateTest : public testing::Test
{
public:
    DFMStyledItemDelegate * delegate = nullptr;
    inhDFileViewHelper* helper = nullptr;
    QListView* listView = new QListView;

    void SetUp() override
    {
        qDebug() << __PRETTY_FUNCTION__;
        listView->setModel(new QStandardItemModel());
        if(!helper) helper = new inhDFileViewHelper(listView);
        if(!delegate) delegate = new inhDFMStyledItemDelegat(helper);
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
    //cause not edit index model,should QModelIndex(QObject(0x0),(-1,-1))
    qDebug() << delegate->editingIndex();
}

TEST_F(DFMStyledItemDelegateTest, editingIndexWidget)
{
    //cause not edit index model,should QModelIndex(QObject(0x0),(-1,-1))
    qDebug() << delegate->editingIndexWidget();
}

TEST_F(DFMStyledItemDelegateTest, sizeHint)
{
    //cause not edit index model,should QModelIndex(QObject(0x0),(-1,-1))
    qDebug() << delegate->sizeHint(QStyleOptionViewItem(),QModelIndex());
}

TEST_F(DFMStyledItemDelegateTest, destroyEditor)
{
    //cause not edit index model,should QModelIndex(QObject(0x0),(-1,-1))
    qDebug() << delegate->sizeHint(QStyleOptionViewItem(),QModelIndex());
}

TEST_F(DFMStyledItemDelegateTest, hasWidgetIndexs)
{
    EXPECT_TRUE(delegate->hasWidgetIndexs().isEmpty());
}


TEST_F(DFMStyledItemDelegateTest, hideAllIIndexWidget)
{
    //model is empty to call
    delegate->hideAllIIndexWidget();
}

TEST_F(DFMStyledItemDelegateTest,hideNotEditingIndexWidget)
{
    //model is empty to call
    delegate->hideNotEditingIndexWidget();
}

TEST_F(DFMStyledItemDelegateTest,commitDataAndCloseActiveEditor)
{
    //model is empty to call
    delegate->commitDataAndCloseActiveEditor();
}

TEST_F(DFMStyledItemDelegateTest,fileNameRect)
{
    //get item filename rect in view, arguments is empty const class;
    qWarning() << delegate->fileNameRect(QStyleOptionViewItem(),QModelIndex());
    //    EXPECT_TRUE(QRect(-1,-1,-1,-1) == delegate->fileNameRect(QStyleOptionViewItem(),QModelIndex()));
}

TEST_F(DFMStyledItemDelegateTest,paintGeomertys)
{
    //overload vritual method return list is empty
    EXPECT_TRUE(delegate->paintGeomertys(QStyleOptionViewItem(),QModelIndex()).isEmpty());
}

TEST_F(DFMStyledItemDelegateTest,iconSizeLevel)
{
    qDebug() << delegate->iconSizeLevel();
}

TEST_F(DFMStyledItemDelegateTest,minimumIconSizeLevel)
{
    qDebug() << delegate->iconSizeLevel();
}

TEST_F(DFMStyledItemDelegateTest,maximumIconSizeLevel)
{
    qDebug() << delegate->iconSizeLevel();
}

TEST_F(DFMStyledItemDelegateTest,increaseIcon)
{
    qDebug() << delegate->iconSizeLevel();
}

TEST_F(DFMStyledItemDelegateTest,decreaseIcon)
{
    qDebug() << delegate->iconSizeLevel();
}

TEST_F(DFMStyledItemDelegateTest,setIconSizeByIconSizeLevel)
{
    qDebug() << delegate->setIconSizeByIconSizeLevel(0);
}

TEST_F(DFMStyledItemDelegateTest,updateItemSizeHint)
{
    delegate->updateItemSizeHint();
}

#include <QPainter>
TEST_F(DFMStyledItemDelegateTest,drawText)
{
    qDebug() << delegate->drawText(QModelIndex(),new QPainter,new QTextLayout(""),
                                   QRectF(),0.0,QBrush());
}

TEST_F(DFMStyledItemDelegateTest,drawTest2)
{
    qDebug() << delegate->drawText(QModelIndex(),new QPainter,"",
                                   QRectF(),0.0,QBrush());
}

#include <QColor>
TEST_F(DFMStyledItemDelegateTest,paintCircleList)
{
    delegate->paintCircleList(new QPainter,QRect(),0.0,{QColor()},QColor());
}

#include <QIcon>
TEST_F(DFMStyledItemDelegateTest,getIconPixmap)
{
    delegate->getIconPixmap(QIcon(),{0,0},0.0);
}


