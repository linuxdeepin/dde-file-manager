#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "../controllers/filecontroller.h"

#include <dboxwidget.h>

#include <QLabel>
#include <QFileSystemModel>
#include <QPushButton>

class IconItem : public DVBoxWidget
{
public:
    explicit IconItem(QWidget *parent = 0) :
        DVBoxWidget(parent)
    {
        icon = new QLabel;
        label = new QLabel;

        label->setWordWrap(true);

        layout()->addWidget(icon, 0, Qt::AlignHCenter);
        layout()->addWidget(label, 0 , Qt::AlignHCenter);
    }

public:
    QLabel *icon;
    QLabel *label;
};

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

        Q_UNUSED(option);
        Q_UNUSED(index);

        QLabel label;

        label.setText(index.data().toString());
        label.setWordWrap(true);
        label.setMaximumWidth(100);
        label.adjustSize();

        QSize size = label.sizeHint();

        size.setWidth(qMin(label.maximumWidth(), size.width()));

        return QSize(100, size.height() + 60);
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

        IconItem *item = new IconItem(parent);

        item->resize(option.rect.size());

        setWidgetData(item, index);

        return item;
    }

    void setWidgetData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        IconItem *item = static_cast<IconItem*>(editor);

        if(!item)
            return;

        item->icon->setPixmap(opt.icon.pixmap(QSize(60, 60)));
        item->label->setMaximumWidth(100);
        item->label->setText(index.data().toString());
        item->label->adjustSize();
    }

    bool viewIsWrapping = true;
};

DFileView::DFileView(QWidget *parent) : DListView(parent)
{
    initUI();
    initDelegate();
    initModel();
    initController();
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
    model()->setRootPath(QUrl::fromLocalFile("/"));
    cd(model()->rootPath());
}

void DFileView::initController()
{
    m_controller = new FileController(this);
}

void DFileView::initConnects()
{
    connect(this, &DListView::doubleClicked,
            this, [this](const QModelIndex &index) {
        setRootIndex(index);
    });

    connect(fileSignalManager, &FileSignalManager::getIcon,
            m_controller, &FileController::getIcon);
    connect(fileSignalManager, &FileSignalManager::getChildren,
            m_controller, &FileController::getChildren);
    connect(fileSignalManager, &FileSignalManager::getChildren, []{
        qDebug () << "get children;";
    });
    connect(fileSignalManager, &FileSignalManager::getIconFinished,
            model(), &DFileSystemModel::updateIcon);
    connect(fileSignalManager, &FileSignalManager::getChildrenFinished,
            model(), &DFileSystemModel::updateChildren);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

void DFileView::back()
{

}

void DFileView::cd(const QString &dir)
{
    QDir::setCurrent(dir);
    setRootIndex(model()->index(QUrl::fromLocalFile(QDir::currentPath())));
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
