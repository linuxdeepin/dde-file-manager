#include "dfileview.h"

#include <libdui/dboxwidget.h>

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
    m_model = new QFileSystemModel(this);
    m_delegate = new ItemDelegate(this);

    setItemDelegate(m_delegate);
    setSpacing(10);
    setResizeMode(QListView::Adjust);
    setCacheBuffer(50);
    setModel(m_model);
    setOrientation(QListView::LeftToRight, true);

    m_model->setRootPath("/");
    setRootIndex(m_model->index("/"));

    connect(this, &DListView::doubleClicked,
            this, [this](const QModelIndex &index) {
        setRootIndex(index);
    });

    setStyleSheet("background: white");
}

DFileView::~DFileView()
{

}

void DFileView::back()
{

}

void DFileView::cd(const QString &dir)
{
    setRootIndex(m_model->index(dir));
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

