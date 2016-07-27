#include "dfileitemdelegate.h"
#include "fileitem.h"
#include "dfilesystemmodel.h"
#include "deditorwidgetmenu.h"

#include "../app/global.h"

#include <QLabel>
#include <QPainter>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>

#define ICON_SPACING 16
#define ICON_MODE_RECT_RADIUS 4
#define LIST_MODE_RECT_RADIUS 2
#define LIST_EDITER_HEIGHT 22
#define LIST_MODE_EDITOR_LEFT_PADDING -9
#define SELECTED_BACKGROUND_COLOR "#2da6f7"
#define ICON_MODE_TEXT_COLOR "#303030"

DFileItemDelegate::DFileItemDelegate(DFileView *parent) :
    QStyledItemDelegate(parent)
{
    expanded_item = new FileIconItem(parent->viewport());
    expanded_item->setAttribute(Qt::WA_TransparentForMouseEvents);
    expanded_item->setProperty("showBackground", true);
    expanded_item->edit->setReadOnly(true);
    expanded_item->canDeferredDelete = false;
    expanded_item->icon->setFixedSize(parent->iconSize());
    /// prevent flash when first call show()
    expanded_item->setFixedWidth(0);

    connect(parent, &DListView::triggerEdit,
            this, [this, parent](const QModelIndex &index) {
        if(index == expanded_index) {
            parent->setIndexWidget(index, 0);
            expanded_item->hide();
            expanded_index = QModelIndex();
            lastAndExpandedInde = QModelIndex();
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
    expanded_item->canDeferredDelete = true;
}

void DFileItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice*)parent()->viewport() != painter->device());

    if (parent()->isCutIndex(index))
        painter->setOpacity(0.3);

    painter->setPen(QColor(ICON_MODE_TEXT_COLOR));

    if (parent()->isIconViewMode()) {
        paintIconItem(painter, option, index, isDragMode, parent()->isActiveIndex(index));
    } else {
        paintListItem(painter, option, index, isDragMode, parent()->isActiveIndex(index));
    }

    painter->setOpacity(1);
}

QSize DFileItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    const QSize &size = parent()->itemSizeHint();

    if (index == lastAndExpandedInde) {
        expanded_item->setFixedWidth(size.width());

        return expanded_item->size();
    }

    return size;
}

QWidget *DFileItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    editing_index = index;

    if (this->parent()->isIconViewMode()) {
        FileIconItem *item = new FileIconItem(parent);

        connect(item, &FileIconItem::inputFocusOut, this, &DFileItemDelegate::onEditWidgetFocusOut);
        connect(item, &FileIconItem::destroyed, this, [this] {
            editing_index = QModelIndex();
        });

        Q_UNUSED(new DEditorWidgetMenu(item->edit))

        return item;
    } else {
        QLineEdit *edit = new QLineEdit(parent);
        edit->setFixedHeight(LIST_EDITER_HEIGHT);

        connect(edit, &QLineEdit::destroyed, this, [this] {
            editing_index = QModelIndex();
        });

        connect(edit, &QLineEdit::textChanged, this, [edit] {
            QSignalBlocker blocker(edit);
            Q_UNUSED(blocker)

            QString text = edit->text();
            const QString old_text = text;

            int text_length = text.length();

            text.remove('/');
            text.remove(QChar(0));

            QVector<uint> list = text.toUcs4();
            int cursor_pos = edit->cursorPosition() - text_length + text.length();

            while (text.toUtf8().size() > MAX_FILE_NAME_CHAR_COUNT) {
                list.removeAt(--cursor_pos);

                text = QString::fromUcs4(list.data(), list.size());
            }

            if (text.count() != old_text.count()) {
                edit->setText(text);
                edit->setCursorPosition(cursor_pos);
            }
        });

        edit->setFrame(false);
        edit->setAttribute(Qt::WA_TranslucentBackground);
        edit->setContentsMargins(0, 0, 0, 0);

        Q_UNUSED(new DEditorWidgetMenu(edit))

        return edit;
    }
}

void DFileItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize &icon_size = parent()->iconSize();

    if(parent()->isIconViewMode()) {
        editor->move(option.rect.topLeft());
        editor->setFixedWidth(option.rect.width());
        editor->setMinimumHeight(option.rect.height());

        FileIconItem *item = qobject_cast<FileIconItem*>(editor);

        if(!item)
            return;

        if(icon_size.width() != item->icon->size().width()) {
            QStyleOptionViewItem opt;

            initStyleOption(&opt, index);

            QPixmap pixmap = opt.icon.pixmap(icon_size, QIcon::Selected);
            QPainter painter(&pixmap);

            /// draw file additional icon

            QList<QRect> cornerGeometryList = getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
            const QList<QIcon> &cornerIconList = parent()->fileAdditionalIcon(index);

            for (int i = 0; i < cornerIconList.count(); ++i) {
                cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i));
            }

            item->icon->setFixedSize(icon_size);
            item->icon->setPixmap(pixmap);
        }
    } else {
        const QList<int> &columnRoleList = parent()->columnRoleList();

        int column_x = 0;

        /// draw icon

        const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

        QRect icon_rect = opt_rect;

        icon_rect.setSize(icon_size);

        column_x = icon_rect.right() + ICON_SPACING + 1;

        QRect rect = opt_rect;

        rect.setLeft(column_x + LIST_MODE_EDITOR_LEFT_PADDING);

        column_x = parent()->columnWidth(0) - 1 - parent()->viewportMargins().left();

        rect.setRight(qMin(column_x, opt_rect.right()));
        rect.setTop(opt_rect.y() + (opt_rect.height() - LIST_EDITER_HEIGHT) / 2);

        editor->setGeometry(rect);
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

        const QSize &icon_size = parent()->iconSize();
        QPixmap pixmap = opt.icon.pixmap(icon_size, QIcon::Selected);
        QPainter painter(&pixmap);

        /// draw file additional icon

        QList<QRect> cornerGeometryList = getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
        const QList<QIcon> &cornerIconList = parent()->fileAdditionalIcon(index);

        for (int i = 0; i < cornerIconList.count(); ++i) {
            cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i));
        }

        item->icon->setPixmap(pixmap);
        item->edit->setPlainText(index.data().toString());
        item->edit->setAlignment(Qt::AlignHCenter);
        item->edit->document()->setTextWidth(parent()->itemSizeHint().width());

        if(item->edit->isReadOnly())
            return;

        const AbstractFileInfoPointer &fileInfo = parent()->model()->fileInfo(index);

        int endPos = -1;

        if (fileInfo) {
            if (fileInfo->isFile()) {
                const QString &suffix = fileInfo->suffix();

                if (!suffix.isEmpty())
                    endPos = item->edit->toPlainText().length() - suffix.length() - 1;
            }
        } else {
            endPos = item->edit->toPlainText().lastIndexOf('.');
        }

        if (endPos == -1) {
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

void DFileItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::destroyEditor(editor, index);

    editing_index = QModelIndex();
}

QString trimmedEnd(QString str)
{
    while (!str.isEmpty()) {
        switch (str.at(str.count() - 1).toLatin1()) {
        case '\t':
        case '\n':
        case '\r':
        case '\v':
        case '\f':
        case ' ':
            str.chop(1);
            break;
        default:
            return str;
        }
    }

    return str;
}

void DFileItemDelegate::paintIconItem(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index, bool isDragMode, bool isActive) const
{
    if((index == expanded_index || index == editing_index) && !isDragMode)
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    if (isActive && !isSelected) {
        QPen pen;
        QRectF rect = opt.rect;
        QPainterPath path;

        rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());

        pen.setColor(SELECTED_BACKGROUND_COLOR);

        path.addRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, QColor(43, 167, 248, 0.50 * 255));
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::black);
    }

    /// init icon geomerty

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    icon_rect.moveLeft(opt.rect.left() + (opt.rect.width() - icon_rect.width()) / 2.0);
    icon_rect.moveTop(opt.rect.top());

    QString str = opt.text;

    /// init file name geometry

    QRect label_rect = opt.rect;

    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);
    label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING);
    label_rect.moveLeft(label_rect.left() + TEXT_PADDING);

    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexCount() < 2;

    if((opt.state & QStyle::State_Selected) && singleSelected) {
        const_cast<DFileItemDelegate*>(this)->hideExpandedIndex();

        int height = 0;

        /// init file name text

        if(m_wordWrapMap.contains(str)) {
            str = m_wordWrapMap.value(str);
            height = m_textHeightMap.value(str);
        } else {
            QString wordWrap_str = Global::wordWrapText(str, label_rect.width(),
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                        &height);

            wordWrap_str = trimmedEnd(wordWrap_str);

            m_wordWrapMap[str] = wordWrap_str;
            m_textHeightMap[wordWrap_str] = height;
            str = wordWrap_str;
        }

        if(height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.

            expanded_index = index;

            setEditorData(expanded_item, index);
            parent()->setIndexWidget(index, expanded_item);

            if (parent()->indexOfRow(index) == parent()->rowCount() - 1) {
                lastAndExpandedInde = index;
            }

            return;
        }
    } else {
        /// init file name text

        if(m_elideMap.contains(str)) {
            str = m_elideMap.value(str);
        } else {
            QString elide_str = Global::elideText(str, label_rect.size(),
                                                  painter->fontMetrics(),
                                                  QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                  opt.textElideMode);

            elide_str = trimmedEnd(elide_str);

            m_elideMap[str] = elide_str;

            str = elide_str;
        }

        if (!singleSelected) {
            const_cast<DFileItemDelegate*>(this)->hideExpandedIndex();
        }
    }

    /// draw icon

    if (isSelected) {
        painter->setPen(Qt::white);
        opt.icon.paint(painter, icon_rect, Qt::AlignCenter, QIcon::Selected);
    } else if (isActive) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size());

        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size()), QColor(0, 0, 0, 255 * 0.1));
        p.end();

        painter->drawPixmap(icon_rect, pixmap);
    } else {
        opt.icon.paint(painter, icon_rect);
    }

    /// draw file additional icon

    QList<QRect> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 3);
    const QList<QIcon> &cornerIconList = parent()->fileAdditionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i));
    }

    /// draw file name label

    if(str.indexOf("\n") >=0 && !str.endsWith("\n")) {
        QTextDocument *doc = m_documentMap.value(str);

        if(!doc) {
            doc = new QTextDocument(str, const_cast<DFileItemDelegate*>(this));

            QTextCursor cursor(doc);
            QTextOption text_option(Qt::AlignHCenter);

            text_option.setWrapMode(QTextOption::NoWrap);
//            doc->setDefaultFont(painter->font());
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

        if (isSelected) {
            QRect rect = opt.rect;

            rect.moveTop(label_rect.top() - TEXT_PADDING);
            rect.setHeight(doc->size().height());

            QPainterPath path;

            path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(path, QColor(SELECTED_BACKGROUND_COLOR));
            painter->restore();
        } else {
            painter->fillRect(label_rect, Qt::transparent);
        }

        painter->save();
        painter->translate(label_rect.left(), label_rect.top() - TEXT_PADDING);
        doc->documentLayout()->draw(painter, ctx);
        painter->restore();
    } else {
        if(isSelected) {
            QRect rect;

            painter->drawText(label_rect, Qt::AlignHCenter, str, &rect);
            rect += QMargins(TEXT_PADDING, TEXT_PADDING, TEXT_PADDING, TEXT_PADDING);

            QPainterPath path;

            path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(path, QColor(SELECTED_BACKGROUND_COLOR));
            painter->restore();
        } else {
            painter->fillRect(label_rect, Qt::transparent);
        }

        painter->drawText(label_rect, Qt::AlignHCenter, str);
    }
}

void DFileItemDelegate::paintListItem(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index, bool isDragMode, bool isActive) const
{
    const QList<int> &columnRoleList = parent()->columnRoleList();

    int column_x = 0;

    QStyleOptionViewItem opt = option;

    initStyleOption(&opt, index);

    opt.rect += QMargins(-LIST_MODE_LEFT_MARGIN, 0, -LIST_MODE_RIGHT_MARGIN, 0);

    /// draw background

    bool drawBackground = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    if (drawBackground) {
        QPainterPath path;

        path.addRoundedRect(opt.rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, QColor(SELECTED_BACKGROUND_COLOR));
        painter->restore();
        painter->setPen(Qt::white);
    } else {
        if (!isDragMode){
            if (index.row() % 2 == 0){
                painter->fillRect(opt.rect, QColor(252, 252, 252));
            }else{
                painter->fillRect(opt.rect, QColor(255, 255, 255));
            }
        }

        if (isActive) {
            QRectF rect = opt.rect;
            QPainterPath path;

            rect += QMarginsF(-0.5, -0.5, -0.5, -0.5);

            path.addRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->fillPath(path, QColor(43, 167, 248, 0.50 * 255));
            painter->setRenderHint(QPainter::Antialiasing, false);
        }
    }

    opt.rect.setLeft(opt.rect.left() + LEFT_PADDING);
    opt.rect.setRight(opt.rect.right() - RIGHT_PADDING);

    /// draw icon

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    icon_rect.moveTop(icon_rect.top() + (opt.rect.bottom() - icon_rect.bottom()) / 2);

    if (isActive) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size());

        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size()), QColor(0, 0, 0, 255 * 0.1));
        p.end();

        painter->drawPixmap(icon_rect, pixmap);
    } else {
        opt.icon.paint(painter, icon_rect);
    }

    /// draw file additional icon

    QList<QRect> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 2);
    const QList<QIcon> &cornerIconList = parent()->fileAdditionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i));
    }

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    column_x = parent()->columnWidth(0) - 1 - parent()->viewportMargins().left();

    rect.setRight(qMin(column_x, opt.rect.right()));

    int role = columnRoleList.at(0);

    if(index != editing_index || role != DFileSystemModel::FileNameRole) {
        /// draw file name label
        const QString &file_name = Global::elideText(index.data(role).toString(), rect.size(),
                                                     painter->fontMetrics(), QTextOption::NoWrap, Qt::ElideRight);

        painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), file_name);
    }

    if(isDragMode)
        return;

    const DFileSystemModel *model = qobject_cast<const DFileSystemModel*>(index.model());

    if (drawBackground) {
        painter->setPen("#e9e9e9");
    } else {
        painter->setPen("#797979");
    }

    for(int i = 1; i < columnRoleList.count(); ++i) {
        int column_width = parent()->columnWidth(i);

        if (column_width <= 0)
            continue;

        QRect rect = opt.rect;

        rect.setLeft(column_x + COLUMU_PADDING);

        if (rect.left() >= rect.right())
            break;

        column_x += parent()->columnWidth(i) - 1;

        rect.setRight(qMin(column_x, opt.rect.right()));

        int role = columnRoleList.at(i);

        QModelIndex tmp_index = model->createIndex(index.row(), model->roleToColumn(role), index.internalId());

        const QString &text = Global::elideText(index.data(role).toString(), rect.size(),
                                                painter->fontMetrics(), QTextOption::NoWrap, Qt::ElideRight);

        painter->drawText(rect, Qt::Alignment(tmp_index.data(Qt::TextAlignmentRole).toInt()), text);
    }

    if (isActive && !drawBackground) {
        QPen pen;
        QRectF rect = opt.rect;

        rect += QMarginsF(-0.5 + LEFT_PADDING, -0.5, -0.5 + RIGHT_PADDING, -0.5);

        pen.setColor(SELECTED_BACKGROUND_COLOR);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::black);
    }
}

QList<QRect> DFileItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const
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

        label_rect.setTop(icon_rect.bottom() + TEXT_PADDING  + ICON_MODE_ICON_SPACING);

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
        geomertys.last().setTop(icon_rect.bottom());
    } else {
        const QList<int> &columnRoleList = parent()->columnRoleList();

        int column_x = 0;

        /// draw icon

        const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

        QRect icon_rect = opt_rect;

        icon_rect.setSize(parent()->iconSize());

        geomertys << icon_rect;

        column_x = icon_rect.right() + ICON_SPACING;

        QRect rect = opt_rect;

        rect.setLeft(column_x);

        column_x = parent()->columnWidth(0) - 1 - parent()->viewportMargins().left();

        rect.setRight(qMin(column_x, opt_rect.right()));

        int role = columnRoleList.at(0);

        /// draw file name label

        geomertys << option.fontMetrics.boundingRect(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                                                     index.data(role).toString());

        for(int i = 1; i < columnRoleList.count(); ++i) {
            QRect rect = opt_rect;

            rect.setLeft(column_x + COLUMU_PADDING);

            if (rect.left() >= rect.right())
                return geomertys;

            column_x += parent()->columnWidth(i) - 1;

            rect.setRight(qMin(column_x, opt_rect.right()));

            int role = columnRoleList.at(i);

            /// draw file name label

            geomertys << option.fontMetrics.boundingRect(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()),
                                                         index.data(role).toString());
        }
    }

    return geomertys;
}

void DFileItemDelegate::hideExpandedIndex()
{
    if (expanded_index.isValid()) {
        parent()->setIndexWidget(expanded_index, 0);
        expanded_item->hide();
        expanded_index = QModelIndex();
        lastAndExpandedInde = QModelIndex();
    }
}

void DFileItemDelegate::hideAllIIndexWidget()
{
    hideExpandedIndex();

    if (editing_index.isValid()) {
        parent()->setIndexWidget(editing_index, 0);

        editing_index = QModelIndex();
    }
}

void DFileItemDelegate::commitDataAndCloseActiveEditor()
{
    QWidget *editor = parent()->indexWidget(editing_index);

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget*, editor));
}

QModelIndex DFileItemDelegate::editingIndex() const
{
    return editing_index;
}

QModelIndex DFileItemDelegate::expandedIndex() const
{
    return expanded_index;
}

FileIconItem *DFileItemDelegate::expandedIndexWidget() const
{
    return expanded_item;
}

QWidget *DFileItemDelegate::editingIndexWidget() const
{
    return parent()->indexWidget(editing_index);
}

bool DFileItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Show) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);

        if(edit) {
            const AbstractFileInfoPointer &fileInfo = parent()->model()->fileInfo(editing_index);

            int endPos = -1;

            if (fileInfo) {
                if (fileInfo->isFile()) {
                    const QString &suffix = fileInfo->suffix();

                    if (!suffix.isEmpty())
                        endPos = edit->text().length() - suffix.length() - 1;
                }
            } else {
                endPos = edit->text().lastIndexOf('.');
            }

            if(endPos == -1)
                edit->selectAll();
            else
                edit->setSelection(0, endPos);
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);

        if (e->key() == Qt::Key_Enter) {
            e->accept();

            QLineEdit *edit = qobject_cast<QLineEdit*>(object);

            if (edit)
                edit->close();

            return true;
        }
    } else if (event->type() == QEvent::FocusOut) {
        onEditWidgetFocusOut();

        return true;
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

void DFileItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    if (parent()->isSelected(index))
        option->state |= QStyle::State_Selected;
    else
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
}

void DFileItemDelegate::onEditWidgetFocusOut()
{
    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->window()
            && qApp->focusWidget() != parent()) {

        hideAllIIndexWidget();
    }
}

QList<QRect> DFileItemDelegate::getCornerGeometryList(const QRect &baseRect, const QSize &cornerSize) const
{
    QList<QRect> list;
    int offset = baseRect.width() / 8;
    const QSize &offset_size = cornerSize / 2;

    list << QRect(QPoint(baseRect.right() - offset - offset_size.width(),
                         baseRect.bottom() - offset - offset_size.height()), cornerSize);
    list << QRect(QPoint(baseRect.left() + offset - offset_size.width(), list.first().top()), cornerSize);
    list << QRect(QPoint(list.at(1).left(), baseRect.top() + offset - offset_size.height()), cornerSize);
    list << QRect(QPoint(list.first().left(), list.at(2).top()), cornerSize);

    return list;
}
