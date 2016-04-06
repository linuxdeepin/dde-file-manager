#include "dfileitemdelegate.h"
#include "fileitem.h"
#include "dfilesystemmodel.h"

#include "../app/global.h"

#include <QLabel>
#include <QPainter>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

DFileItemDelegate::DFileItemDelegate(DFileView *parent) :
    QStyledItemDelegate(parent)
{
    focus_item = new FileIconItem(parent->viewport());
    focus_item->setAttribute(Qt::WA_TransparentForMouseEvents);
    focus_item->setProperty("showBackground", true);
    focus_item->edit->setReadOnly(true);
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

DFileItemDelegate::~DFileItemDelegate()
{
    focus_item->canDeferredDelete = true;
}

void DFileItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if(parent()->isIconViewMode()) {
        paintIconItem(painter, option, index);
    } else {
        paintListItem(painter, option, index);
    }
}

QSize DFileItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    if(parent()->isIconViewMode()) {
        int width = parent()->iconSize().width() * 1.8;

        return QSize(width, width * 1.1);
    } else {
        return QSize(-1, 30);
    }
}

QWidget *DFileItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
   if(this->parent()->isIconViewMode()) {
       FileIconItem *item = new FileIconItem(parent);

       editing_index = index;

       connect(item, &FileIconItem::destroyed, this, [this] {
           editing_index = QModelIndex();
       });

       return item;
   } else {
       QLineEdit *edit = new QLineEdit(parent);

       editing_index = index;

       connect(edit, &QLineEdit::destroyed, this, [this] {
           editing_index = QModelIndex();
       });

       edit->setFrame(false);
       edit->setAttribute(Qt::WA_TranslucentBackground);
       edit->setContentsMargins(-3, 0, 0, 0);

       return edit;
   }
}

void DFileItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(parent()->isIconViewMode()) {
        editor->move(option.rect.topLeft());
        editor->setFixedWidth(option.rect.width());
        editor->setMinimumHeight(option.rect.height());

        FileIconItem *item = qobject_cast<FileIconItem*>(editor);

        if(!item)
            return;

        if(parent()->iconSize().width() != item->icon->size().width()) {
            QStyleOptionViewItem opt;

            initStyleOption(&opt, index);

            item->icon->setFixedSize(parent()->iconSize());
            item->icon->setPixmap(opt.icon.pixmap(parent()->iconSize()));
        }
    } else {
        const QList<int> &columnRoleList = parent()->columnRoleList();

        int column_x = 0;

        /// draw icon

        QRect icon_rect = option.rect;

        icon_rect.setSize(parent()->iconSize());

        column_x = icon_rect.right() + 10;

        QRect rect = option.rect;

        rect.setLeft(column_x);

        column_x = parent()->columnWidth(0);

        rect.setRight(column_x);

        int role = columnRoleList.at(0);

        if(role == DFileSystemModel::FileNameRole) {
            editor->setGeometry(rect);
        }

        for(int i = 1; i < columnRoleList.count(); ++i) {
            QRect rect = option.rect;

            rect.setLeft(column_x);

            column_x += parent()->columnWidth(i);

            rect.setRight(column_x);

            int role = columnRoleList.at(i);

            if(role == DFileSystemModel::FileNameRole) {
                editor->setGeometry(rect);
                return;
            }
        }
    }
}

void DFileItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(parent()->isIconViewMode()) {
        FileIconItem *item = qobject_cast<FileIconItem*>(editor);

        if(!item)
            return;

        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        item->icon->setPixmap(opt.icon.pixmap(parent()->iconSize()));
        item->edit->setPlainText(index.data().toString());
        item->edit->setAlignment(Qt::AlignHCenter);
        item->edit->document()->setTextWidth(parent()->iconSize().width() * 1.8);
        item->edit->setFixedSize(item->edit->document()->size().toSize());

        if(item->edit->isReadOnly())
            return;

        int endPos = item->edit->toPlainText().lastIndexOf('.');

        if(endPos == -1) {
            item->edit->selectAll();
        } else {
            QTextCursor cursor = item->edit->textCursor();

            cursor.setPosition(0);
            cursor.setPosition(endPos, QTextCursor::KeepAnchor);

            item->edit->setTextCursor(cursor);
        }
    } else {
        QLineEdit *edit = qobject_cast<QLineEdit*>(editor);

        if(!edit)
            return;

        const QString &text = index.data(DFileSystemModel::FileNameRole).toString();

        edit->setText(text);
    }
}

void DFileItemDelegate::paintIconItem(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if(index == focus_index || index == editing_index)
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    /// init icon geomerty

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    icon_rect.moveCenter(opt.rect.center());
    icon_rect.moveTop(opt.rect.top());

    QString str = opt.text;

    if(str.isEmpty()) {
        /// draw icon

        opt.icon.paint(painter, icon_rect);

        return;
    }

    /// init file name geometry

    QRect label_rect = opt.rect;

    label_rect.setTop(icon_rect.bottom() + 2);
    label_rect.setWidth(opt.rect.width() * 0.91);
    label_rect.moveLeft(label_rect.left() + (opt.rect.width() - label_rect.width()) / 2.0);

    /// if has focus show all file name else show elide file name.

    if((opt.state & QStyle::State_HasFocus) && parent()->selectedIndexCount() < 2) {
        if(focus_index.isValid()) {
            parent()->setIndexWidget(focus_index, 0);
            focus_item->hide();
            focus_index = QModelIndex();
        }

        int height = 0;

        /// init file name text

        if(m_wordWrapMap.contains(str)) {
            str = m_wordWrapMap.value(str);
            height = m_textHeightMap.value(str);
        } else {
            QString wordWrap_str = Global::wordWrapText(str, label_rect.width(),
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                        &height);

            m_wordWrapMap[str] = wordWrap_str;
            m_textHeightMap[wordWrap_str] = height;
            str = wordWrap_str;
        }

        if(height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.

            focus_index = index;

            setEditorData(focus_item, index);
            parent()->setIndexWidget(index, focus_item);

            return;
        }
    } else {
        /// init file name text

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

    /// draw background

    if((opt.state & QStyle::State_Selected) && opt.showDecorationSelected) {
        QPainterPath path;

        path.addRoundedRect(opt.rect, 5, 5);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, QColor("#2da6f7"));
        painter->setPen(Qt::white);
    } else {
        painter->setPen(Qt::black);
    }

    /// draw icon and file name label

    opt.icon.paint(painter, icon_rect);

    if(str.indexOf("\n") >=0 && !str.endsWith("\n")) {
        QTextDocument *doc = m_documentMap.value(str);

        if(!doc) {
            doc = new QTextDocument(str, const_cast<DFileItemDelegate*>(this));

            QTextCursor cursor(doc);
            QTextOption text_option(Qt::AlignHCenter);

            text_option.setWrapMode(QTextOption::NoWrap);
            doc->setDefaultFont(painter->font());
            doc->setDefaultTextOption(text_option);
            doc->setTextWidth(label_rect.width());
            cursor.movePosition(QTextCursor::Start);

            do {
                QTextBlockFormat format = cursor.blockFormat();

                format.setLineHeight(TEXT_LINE_HEIGHT, QTextBlockFormat::FixedHeight);
                cursor.setBlockFormat(format);
            } while (cursor.movePosition(QTextCursor::NextBlock));

            m_documentMap[str] = doc;
        }

        QAbstractTextDocumentLayout::PaintContext ctx;

        ctx.palette.setColor(QPalette::Text, painter->pen().color());
        ctx.clip.setSize(label_rect.size());

        painter->save();
        painter->translate(opt.rect.left(), label_rect.top() - 3);
        doc->documentLayout()->draw(painter, ctx);
        painter->restore();
    } else {
        painter->drawText(label_rect, Qt::AlignHCenter, str);
    }
}

void DFileItemDelegate::paintListItem(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    const QList<int> &columnRoleList = parent()->columnRoleList();

    int column_x = 0;

    QStyleOptionViewItem opt = option;

    initStyleOption(&opt, index);

    /// draw background

    if((opt.state & QStyle::State_Selected) && opt.showDecorationSelected) {
        painter->fillRect(opt.rect, QColor("#2da6f7"));
        painter->setPen(Qt::white);
    } else {
        painter->setPen(Qt::black);
    }

    /// draw icon

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    opt.icon.paint(painter, icon_rect);

    column_x = icon_rect.right() + 10;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    column_x = parent()->columnWidth(0);

    rect.setRight(column_x);

    int role = columnRoleList.at(0);

    if(index != editing_index || role != DFileSystemModel::FileNameRole) {
    /// draw file name label

    painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                      index.data(role).toString());
    }

    for(int i = 1; i < columnRoleList.count(); ++i) {
        QRect rect = opt.rect;

        rect.setLeft(column_x + 5);

        column_x += parent()->columnWidth(i);

        rect.setRight(column_x);

        int role = columnRoleList.at(i);

        if(index != editing_index || role != DFileSystemModel::FileNameRole) {
            /// draw file name label

            painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                              index.data(role).toString());
        }
    }
}

QList<QRect> DFileItemDelegate::paintGeomertyss(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QRect> geomertys;

    if(parent()->isIconViewMode()) {
        /// init icon geomerty

        QRect icon_rect = option.rect;

        icon_rect.setSize(parent()->iconSize());
        icon_rect.moveCenter(option.rect.center());
        icon_rect.moveTop(option.rect.top());

        geomertys << icon_rect;

        QString str = index.data(Qt::DisplayRole).toString();

        if(str.isEmpty()) {
            return geomertys;
        }

        /// init file name geometry

        QRect label_rect = option.rect;

        label_rect.setTop(icon_rect.bottom() + 10);

        /// if has focus show all file name else show elide file name.
        /// init file name text

        if(m_elideMap.contains(str)) {
            str = m_elideMap.value(str);
        } else {
            QString elide_str = Global::elideText(str, label_rect.size(),
                                                  option.fontMetrics,
                                                  QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                  option.textElideMode);

            m_elideMap[str] = elide_str;

            str = elide_str;
        }

        /// draw icon and file name label

        geomertys << option.fontMetrics.boundingRect(label_rect, Qt::AlignHCenter, str);
    } else {
        const QList<int> &columnRoleList = parent()->columnRoleList();

        int column_x = 0;

        /// draw icon

        QRect icon_rect = option.rect;

        icon_rect.setSize(parent()->iconSize());

        geomertys << icon_rect;

        column_x = icon_rect.right() + 10;

        QRect rect = option.rect;

        rect.setLeft(column_x);

        column_x = parent()->columnWidth(0);

        rect.setRight(column_x);

        int role = columnRoleList.at(0);

        /// draw file name label

        geomertys << option.fontMetrics.boundingRect(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                                                     index.data(role).toString());
        for(int i = 1; i < columnRoleList.count(); ++i) {
            QRect rect = option.rect;

            rect.setLeft(column_x);

            column_x += parent()->columnWidth(i);

            rect.setRight(column_x);

            int role = columnRoleList.at(i);

            /// draw file name label

            geomertys << option.fontMetrics.boundingRect(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                                                         index.data(role).toString());
        }
    }

    return geomertys;
}

void DFileItemDelegate::hideAllIIndexWidget()
{
    if(focus_index.isValid()) {
        parent()->setIndexWidget(focus_index, 0);
        focus_item->hide();
        focus_index = QModelIndex();
    }

    if(editing_index.isValid()) {
        parent()->setIndexWidget(editing_index, 0);
    }
}

void DFileItemDelegate::commitDataAndCloseActiveEditor()
{
    QWidget *editor = parent()->indexWidget(editing_index);

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::QueuedConnection, Q_ARG(QWidget*, editor));
}

QModelIndex DFileItemDelegate::editingIndex() const
{
    return editing_index;
}

bool DFileItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Show) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);

        if(edit) {
            int endPos = edit->text().lastIndexOf('.');

            if(endPos == -1)
                edit->selectAll();
            else
                edit->setSelection(0, endPos);
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}
