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
#include <QWheelEvent>
#include <QDesktopServices>

class IconItem : public DVBoxWidget
{
public:
    explicit IconItem(QWidget *parent = 0) :
        DVBoxWidget(parent)
    {
        icon = new QLabel;
        edit = new QTextEdit;

        icon->setAlignment(Qt::AlignCenter);
        edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        edit->setAlignment(Qt::AlignHCenter);
        edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        layout()->addWidget(icon, 0, Qt::AlignHCenter);
        addWidget(edit);
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
        focus_item = new IconItem(parent->viewport());
        focus_item->setAttribute(Qt::WA_TransparentForMouseEvents);
        focus_item->canDeferredDelete = false;
        focus_item->icon->setFixedSize(parent->iconSize());
        /// prevent flash when first call show()
        focus_item->setFixedWidth(0);

        connect(parent, &DListView::triggerEdit,
                this, [this, parent](const QModelIndex &index) {
            if(index == focus_index) {
                parent->setIndexWidget(index, 0);
                focus_item->hide();
                focus_index = QModelIndex();
                parent->edit(index);
            }
        });

        connect(parent, &DListView::iconSizeChanged,
                this, [this] {
            m_elideMap.clear();
            m_wordWrapMap.clear();
            m_textHeightMap.clear();
        });
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

        icon_rect.setSize(parent()->iconSize());
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
                parent()->setIndexWidget(focus_index, 0);
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
                parent()->setIndexWidget(index, focus_item);

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
        return parent()->iconSize() * 1.8;
    }

    // editing

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        IconItem *item = new IconItem(parent);

        editing_index = index;

        connect(item, &IconItem::destroyed, this, [this] {
            editing_index = QModelIndex();
        });

        return item;
    }

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const Q_DECL_OVERRIDE
    {
        editor->move(option.rect.topLeft());
        editor->setFixedWidth(option.rect.width());

        IconItem *item = static_cast<IconItem*>(editor);

        if(!item)
            return;

        item->icon->setFixedSize(parent()->iconSize());
    }

    void setEditorData(QWidget * editor, const QModelIndex & index) const Q_DECL_OVERRIDE
    {
        IconItem *item = static_cast<IconItem*>(editor);

        if(!item)
            return;

        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        item->icon->setPixmap(opt.icon.pixmap(parent()->iconSize()));
        item->edit->setPlainText(index.data().toString());
        item->edit->setAlignment(Qt::AlignHCenter);
        item->edit->document()->setTextWidth(parent()->iconSize().width() * 1.8);
        item->edit->setFixedSize(item->edit->document()->size().toSize());
    }

    IconItem *focus_item;

    mutable QMap<QString, QString> m_elideMap;
    mutable QMap<QString, QString> m_wordWrapMap;
    mutable QMap<QString, int> m_textHeightMap;
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
    setIconSize(QSize(60, 60));

    setTextElideMode(Qt::ElideMiddle);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionRectVisible(true);
    setEditTriggers(QListView::EditKeyPressed);
}

void DFileView::initDelegate()
{
    m_delegate = new ItemDelegate(this);

    setItemDelegate(m_delegate);
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setRootIndex(model()->setRootPath(QUrl::fromLocalFile("/").toString(QUrl::EncodeUnicode)));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if(model()->hasChildren(index)){
            emit fileSignalManager->currentUrlChanged(model()->getUrlByIndex(index));
        } else {
            QDesktopServices::openUrl(QUrl(model()->getUrlByIndex(index)));
        }
    });
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DFileView::cd);
    connect(fileSignalManager, &FileSignalManager::iconChanged,
            model(), &DFileSystemModel::updateIcon);
    connect(fileSignalManager, &FileSignalManager::childrenChanged,
            model(), &DFileSystemModel::updateChildren);
    connect(fileSignalManager, &FileSignalManager::refreshFolder,
            model(), &DFileSystemModel::refresh);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

QString DFileView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

void DFileView::cd(const QString &url)
{
    qDebug() << "cd: current url:" << currentUrl() << "to url:" << url;

    QModelIndex index = model()->index(url);

    if(!index.isValid())
        index = model()->setRootPath(url);

    setRootIndex(index);
}

void DFileView::switchListMode()
{
    if(isWrapping()) {
        setOrientation(QListView::TopToBottom, false);
    } else {
        setOrientation(QListView::LeftToRight, true);
    }
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << event << indexAt(event->pos());
    QModelIndex index = indexAt(event->pos());
    QMenu* menu;
    if (index.isValid()){
        menu = m_fileMenuManager->genereteMenuByFileType("File");
        menu->setProperty("url", "/home");
    }else{
        menu = m_fileMenuManager->genereteMenuByFileType("Space");
    }
    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();
    menu->deleteLater();
}

void DFileView::wheelEvent(QWheelEvent *event)
{
    if(ctrlIsPressed) {
        if(event->angleDelta().y() > 0) {
            setIconSize(iconSize() * 1.1);
        } else {
            setIconSize(iconSize() * 0.9);
        }
    }

    DListView::wheelEvent(event);
}

void DFileView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        ctrlIsPressed = true;
    }

    DListView::keyPressEvent(event);
}

void DFileView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        ctrlIsPressed = false;
    }

    DListView::keyReleaseEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}
