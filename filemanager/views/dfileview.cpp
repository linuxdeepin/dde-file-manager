#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "fileitem.h"
#include <dboxwidget.h>

#include <QLabel>
#include <QFileSystemModel>
#include <QPushButton>


class ItemDelegate : public DListItemDelegate
{
public:
    ItemDelegate(QObject *parent = 0) : DListItemDelegate(parent)
    {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(viewIsWrapping)
            DListItemDelegate::paint(painter, option, index);
        else
            QStyledItemDelegate::paint(painter, option, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(!viewIsWrapping)
            return DListItemDelegate::sizeHint(option, index);

        return QSize(100, 100);
    }

    // editing
    QWidget *createWidget(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(!viewIsWrapping)
            return 0;

        Q_UNUSED(option);
        Q_UNUSED(index);

        QStyleOptionViewItem opt = option;

        initStyleOption(&opt, index);

        FileItem *item = new FileItem("", "", parent);

        item->resize(option.rect.size());

        setWidgetData(item, index);

        return item;
    }

    void setWidgetData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        FileItem *fileItem = static_cast<FileItem*>(editor);
        if(!fileItem)
            return;
        fileItem->setFileIcon(opt.icon.pixmap(QSize(60, 60)));
        fileItem->setFileName(index.data().toString());
    }

    bool viewIsWrapping = true;
};

DFileView::DFileView(QWidget *parent) : DListView(parent)
{
    initUI();
    initDelegate();
    initModel();
    initConnects();
}

DFileView::~DFileView()
{

}

void DFileView::initUI()
{
    setSpacing(10);
    setResizeMode(QListView::Adjust);
    setCacheBuffer(50);
    setOrientation(QListView::LeftToRight, true);
    setStyleSheet("background: white");
}

void DFileView::initDelegate()
{
    m_delegate = new ItemDelegate(this);

    setItemDelegate(m_delegate);
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setRootIndex(model()->setRootPath(QUrl::fromLocalFile("/")));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if(model()->hasChildren(index)){
            emit fileSignalManager->currentUrlChanged(model()->getUrlByIndex(index));
        }
    });
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DFileView::cd);
    connect(fileSignalManager, &FileSignalManager::iconChanged,
            model(), &DFileSystemModel::updateIcon);
    connect(fileSignalManager, &FileSignalManager::childrenChanged,
            model(), &DFileSystemModel::updateChildren);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

QUrl DFileView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

void DFileView::cd(const QUrl &url)
{
    qDebug() << url;
    setRootIndex(model()->index(url));
}

void DFileView::switchListMode()
{
    if(isWrapping()) {
        setOrientation(QListView::TopToBottom, false);
        clear();
    } else {
        setOrientation(QListView::LeftToRight, true);
    }

    m_delegate->viewIsWrapping = isWrapping();
}
