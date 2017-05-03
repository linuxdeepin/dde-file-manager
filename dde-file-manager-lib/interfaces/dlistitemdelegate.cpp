/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dlistitemdelegate.h"
#include "dfileviewhelper.h"
#include "app/define.h"
#include "dfilesystemmodel.h"
#include "private/dstyleditemdelegate_p.h"

#include <QLabel>
#include <QPainter>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>

#define ICON_SPACING 16
#define LIST_MODE_RECT_RADIUS 2
#define LIST_EDITER_HEIGHT 22
#define LIST_MODE_EDITOR_LEFT_PADDING -3
#define LIST_VIEW_ICON_SIZE 28

class DListItemDelegatePrivate : public DStyledItemDelegatePrivate
{
public:
    DListItemDelegatePrivate(DListItemDelegate *qq)
        : DStyledItemDelegatePrivate(qq) {}
};

DListItemDelegate::DListItemDelegate(DFileViewHelper *parent) :
    DStyledItemDelegate(parent)
{
    parent->parent()->setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
}

void DListItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const DListItemDelegate);

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice*)parent()->parent()->viewport() != painter->device());
    bool isDropTarget = parent()->isDropTarget(index);
    bool isEnabled = option.state & QStyle::State_Enabled;

    if (parent()->isCut(index))
        painter->setOpacity(0.3);

    const QList<int> &columnRoleList = parent()->columnRoleList();

    int column_x = 0;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont old_font = opt.font;

    if (old_font != opt.font) {
        QWidget *editing_widget = editingIndexWidget();

        if (editing_widget)
            editing_widget->setFont(opt.font);

        const_cast<DListItemDelegate*>(this)->updateItemSizeHint();
    }

    old_font = opt.font;

    opt.rect += QMargins(-LIST_MODE_LEFT_MARGIN, 0, -LIST_MODE_RIGHT_MARGIN, 0);

    /// draw background

    bool drawBackground = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    if (drawBackground) {
        QPainterPath path;

        path.addRoundedRect(opt.rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, opt.backgroundBrush);
        painter->restore();
    } else {
        if (!isDragMode){
            if (index.row() % 2 == 0){
                painter->fillRect(opt.rect, QColor(0, 0, 0, 3));
            }else{
                painter->fillRect(opt.rect, QColor(255, 255, 255, 0));
            }
        }

        if (isDropTarget) {
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

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveTop(icon_rect.top() + (opt.rect.bottom() - icon_rect.bottom()) / 2);

    if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size());

        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size()), QColor(0, 0, 0, 255 * 0.1));
        p.end();

        painter->drawPixmap(icon_rect, pixmap);
    } else {
        opt.icon.paint(painter, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    /// draw file additional icon

    QList<QRect> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 2);
    const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i));
    }

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    column_x = parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

    rect.setRight(qMin(column_x, opt.rect.right()));

    int role = columnRoleList.at(0);

    if (index != d->editingIndex || (role != DFileSystemModel::FileNameRole && role != DFileSystemModel::FileDisplayNameRole)) {
        /// draw file name label
        const QString &file_name = DFMGlobal::elideText(index.data(role).toString().remove('\n'),
                                                        rect.size(), QTextOption::NoWrap,
                                                        opt.font, Qt::ElideRight,
                                                        d->textLineHeight);

        painter->setPen(opt.palette.color(drawBackground ? QPalette::BrightText : QPalette::Text));
        painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), file_name);
    }

    if(isDragMode)
        return;

    const DFileSystemModel *model = qobject_cast<const DFileSystemModel*>(index.model());
    painter->setPen(opt.palette.color(QPalette::Inactive, QPalette::Text));

    for(int i = 1; i < columnRoleList.count(); ++i) {
        int column_width = parent()->columnWidth(i);

        if (column_width <= 0)
            continue;

        QRect rect = opt.rect;

        rect.setLeft(column_x + COLUMU_PADDING);

        if (rect.left() >= rect.right())
            break;

        column_x += column_width;

        rect.setRight(qMin(column_x, opt.rect.right()));

        int role = columnRoleList.at(i);

        QModelIndex tmp_index = model->createIndex(index.row(), model->roleToColumn(role), index.internalId());

        const QString &text = DFMGlobal::elideText(index.data(role).toString(), rect.size(),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideRight, d->textLineHeight);

        painter->drawText(rect, Qt::Alignment(tmp_index.data(Qt::TextAlignmentRole).toInt()), text);
    }

    if (isDropTarget && !drawBackground) {
        QPen pen;
        QRectF rect = opt.rect;

        rect += QMarginsF(-0.5 + LEFT_PADDING, -0.5, -0.5 + RIGHT_PADDING, -0.5);

        pen.setColor(opt.backgroundBrush.color());

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::black);
    }

    painter->setOpacity(1);
}

QWidget *DListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_D(const DListItemDelegate);

    d->editingIndex = index;

    QLineEdit *edit = new QLineEdit(parent);
    edit->setFixedHeight(LIST_EDITER_HEIGHT);
    edit->setObjectName("DListItemDelegate_Editor");

    connect(edit, &QLineEdit::destroyed, this, [this, d] {
        d->editingIndex = QModelIndex();
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

    return edit;
}

void DListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    const QSize &icon_size = parent()->parent()->iconSize();
    int column_x = 0;

    /// draw icon

    const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

    QRect icon_rect = opt_rect;

    icon_rect.setSize(icon_size);

    column_x = icon_rect.right() + ICON_SPACING + 1;

    QRect rect = opt_rect;

    rect.setLeft(column_x + LIST_MODE_EDITOR_LEFT_PADDING);

    column_x = parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

    rect.setRight(qMin(column_x, opt_rect.right()));
    rect.setTop(opt_rect.y() + (opt_rect.height() - LIST_EDITER_HEIGHT) / 2);

    editor->setGeometry(rect);
}

void DListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(editor);

    if(!edit)
        return;

    const QString &text = index.data(DFileSystemModel::FileNameRole).toString();

    edit->setText(text);
}

QList<QRect> DListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QRect> geomertys;
    const QList<int> &columnRoleList = parent()->columnRoleList();
    int column_x = 0;

    /// draw icon

    const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

    QRect icon_rect = opt_rect;

    icon_rect.setSize(parent()->parent()->iconSize());

    geomertys << icon_rect;

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt_rect;

    rect.setLeft(column_x);

    column_x = parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

    rect.setRight(qMin(column_x, opt_rect.right()));

    int role = columnRoleList.at(0);

    /// draw file name label

    rect.setWidth(qMin(rect.width(), option.fontMetrics.width(index.data(role).toString(), -1, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()))));
    geomertys << rect;

    for(int i = 1; i < columnRoleList.count(); ++i) {
        QRect rect = opt_rect;

        rect.setLeft(column_x + COLUMU_PADDING);

        if (rect.left() >= rect.right())
            return geomertys;

        column_x += parent()->columnWidth(i) - 1;

        rect.setRight(qMin(column_x, opt_rect.right()));

        int role = columnRoleList.at(i);

        /// draw file name label

        rect.setWidth(qMin(rect.width(), option.fontMetrics.width(index.data(role).toString(), -1, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()))));
        geomertys << rect;
    }

    return geomertys;
}

void DListItemDelegate::updateItemSizeHint()
{
    Q_D(DListItemDelegate);

    d->textLineHeight = parent()->parent()->fontMetrics().height();
    d->itemSizeHint = QSize(-1, qMax(int(parent()->parent()->iconSize().height() * 1.1), d->textLineHeight));
}

bool DListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    Q_D(DListItemDelegate);

    if(event->type() == QEvent::Show) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);

        if(edit) {
            const QString &selectionWhenEditing = parent()->baseName(d->editingIndex);
            int endPos = selectionWhenEditing.isEmpty() ? -1 : selectionWhenEditing.length();

            if(endPos == -1)
                edit->selectAll();
            else
                edit->setSelection(0, endPos);
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            QLineEdit *edit = qobject_cast<QLineEdit*>(object);

            if (edit) {
                edit->close();
                edit->parentWidget()->setFocus();
            }

            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}
