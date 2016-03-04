#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "fileitem.h"
#include "filemenumanager.h"

#include <dboxwidget.h>

#include <QLabel>
#include <QFileSystemModel>
#include <QPushButton>
#include <QMenu>
#include <QTextEdit>
#include <QPainter>

class IconItem : public DVBoxWidget
{
public:
    explicit IconItem(QWidget *parent = 0) :
        DVBoxWidget(parent)
    {
        icon = new QLabel;
        edit = new QTextEdit;
        edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        edit->setAlignment(Qt::AlignHCenter);
        edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        layout()->addWidget(icon, 0, Qt::AlignHCenter);
        layout()->addWidget(edit);
    }

protected:
    bool event(QEvent *ee)
    {
        if(!canDeferredDelete && ee->type() == QEvent::DeferredDelete) {
            ee->accept();
            return true;
        }

        return DVBoxWidget::event(ee);
    }

public:
    bool canDeferredDelete = true;
    QLabel *icon;
    QTextEdit *edit;
};

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(DListView *parent = 0) : QStyledItemDelegate(parent)
    {
        focus_item = new IconItem;
        focus_item->setAttribute(Qt::WA_TransparentForMouseEvents);
        focus_item->canDeferredDelete = false;
    }

    inline DListView *parent() const
    {
        return qobject_cast<DListView*>(QStyledItemDelegate::parent());
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(index == focus_index || index == editing_index)
            return;

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QRect icon_rect = opt.rect;

        icon_rect.setHeight(60);
        icon_rect.setWidth(60);
        icon_rect.moveCenter(opt.rect.center());
        icon_rect.moveTop(opt.rect.top());

        QRect label_rect = opt.rect;

        label_rect.setTop(icon_rect.bottom() + 10);
        painter->setFont(opt.font);

        QString str = index.data(Qt::DisplayRole).toString();

        if(str.isEmpty()) {
            opt.icon.paint(painter, icon_rect);

            return;
        }

        if(opt.state & QStyle::State_HasFocus) {
            if(focus_index.isValid()) {
                parent()->QListView::setIndexWidget(focus_index, 0);
                focus_item->hide();
                focus_index = QModelIndex();
            }


            int height = 0;

            if(m_wordWrapMap.contains(str)) {
                str = m_wordWrapMap.value(str);
                height = m_textHeightMap.value(str);
            } else {
                QString wordWrap_str = Global::wordWrapText(str, label_rect.width(), opt.fontMetrics,
                                                            QTextOption::WrapAtWordBoundaryOrAnywhere, &height);

                m_wordWrapMap[str] = wordWrap_str;
                m_textHeightMap[wordWrap_str] = height;
                str = wordWrap_str;
            }

            if(height > label_rect.height()) {
                focus_index = index;

                setEditorData(focus_item, index);
                parent()->QListView::setIndexWidget(index, focus_item);

                return;
            }
        } else {
            if(m_elideMap.contains(str)) {
                str = m_elideMap.value(str);
            } else {
                QString elide_str = Global::elideText(str, label_rect.size(),
                                                      opt.fontMetrics,
                                                      QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                      opt.textElideMode);

                m_elideMap[str] = elide_str;

                str = elide_str;
            }
        }

        if((opt.state & QStyle::State_Selected) && opt.showDecorationSelected) {
            QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                          ? QPalette::Normal : QPalette::Disabled;
            QColor backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                         ? QPalette::Highlight : QPalette::Window);

            painter->fillRect(opt.rect, backgroundColor);
        }

        opt.icon.paint(painter, icon_rect);
        painter->drawText(label_rect, Qt::AlignHCenter, str);
    }

    QSize sizeHint(const QStyleOptionViewItem &,
                   const QModelIndex &) const Q_DECL_OVERRIDE
    {
        return QSize(100, 100);
    }

    // editing

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        IconItem *item = new IconItem(parent);

        focus_item->hide();
        editing_index = index;

        connect(item, &IconItem::destroyed, this, [this] {
            editing_index = QModelIndex();
        });

        return item;
    }

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const Q_DECL_OVERRIDE
    {
        editor->setFixedWidth(option.rect.width());
        editor->move(option.rect.topLeft());
    }

    void setEditorData(QWidget * editor, const QModelIndex & index) const Q_DECL_OVERRIDE
    {
        IconItem *item = static_cast<IconItem*>(editor);

        if(!item)
            return;

        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        item->icon->setPixmap(opt.icon.pixmap(QSize(60, 60)));
        item->edit->setPlainText(index.data().toString());
        item->edit->setAlignment(Qt::AlignHCenter);
    }

    bool viewIsWrapping = true;
    mutable QMap<QString, QString> m_elideMap;
    mutable QMap<QString, QString> m_wordWrapMap;
    mutable QMap<QString, int> m_textHeightMap;
    IconItem *focus_item;
    mutable QModelIndex focus_index;
    mutable QModelIndex editing_index;
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
    m_fileMenuManager = new FileMenuManager(this);
    setSpacing(10);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setStyleSheet("background: white");

    setTextElideMode(Qt::ElideMiddle);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionRectVisible(true);
    setEditTriggers(QAbstractItemView::EditKeyPressed);
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
    } else {
        setOrientation(QListView::LeftToRight, true);
    }

    m_delegate->viewIsWrapping = isWrapping();
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << event;
    QMenu* menu = m_fileMenuManager->genereteMenuByFileType("File");
    menu->setProperty("url", "/home");
    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();
}
