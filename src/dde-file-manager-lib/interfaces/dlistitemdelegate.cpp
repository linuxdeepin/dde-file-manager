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
#include "dfmapplication.h"
#include "controllers/vaultcontroller.h"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainterPath>

#define ICON_SPACING 16
#define LIST_MODE_RECT_RADIUS 2
#define LIST_EDITER_HEIGHT 22
#define LIST_MODE_EDITOR_LEFT_PADDING -3
#define LIST_VIEW_ICON_SIZE 24

DFM_USE_NAMESPACE

class DListItemDelegatePrivate : public DFMStyledItemDelegatePrivate
{
public:
    explicit DListItemDelegatePrivate(DListItemDelegate *qq)
        : DFMStyledItemDelegatePrivate(qq) {}
};

DListItemDelegate::DListItemDelegate(DFileViewHelper *parent) :
    DFMStyledItemDelegate(parent)
{
    parent->parent()->setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
}

void DListItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const DListItemDelegate);

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice *)parent()->parent()->viewport() != painter->device());
    bool isDropTarget = parent()->isDropTarget(index);
    bool isEnabled = option.state & QStyle::State_Enabled;

    if (parent()->isTransparent(index)) {
        painter->setOpacity(0.3);
    }

    const QList<int> &columnRoleList = parent()->columnRoleList();

    int column_x = 0;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont old_font = opt.font;

    if (old_font != opt.font) {
        QWidget *editing_widget = editingIndexWidget();

        if (editing_widget) {
            editing_widget->setFont(opt.font);
        }

        const_cast<DListItemDelegate *>(this)->updateItemSizeHint();
    }

    old_font = opt.font;

    opt.rect += QMargins(-LIST_MODE_LEFT_MARGIN, 0, -LIST_MODE_RIGHT_MARGIN, 0);

    /// draw background
    //列表拖拽时要绘制活动色
    bool drawBackground =/* !isDragMode &&*/ (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    QPalette::ColorGroup cg = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    QPalette::ColorRole colorRole = QPalette::Background;
    if ((isSelected || isDropTarget)) {
        colorRole = QPalette::Highlight;
    }

    if (drawBackground) {
        QPainterPath path;
        path.addRoundedRect(opt.rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->restore();
    } else if (isDropTarget) {
        QRectF rect = opt.rect;
        QPainterPath path;
        rect += QMarginsF(-0.5, -0.5, -0.5, -0.5);
        path.addRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->setRenderHint(QPainter::Antialiasing, false);
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
        paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    /// draw file additional icon

    QList<QRectF> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 2);
    const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        if (cornerIconList.at(i).isNull()) {
            continue;
        }
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i).toRect());
    }

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    column_x = option.rect.x() + parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

    rect.setRight(qMin(column_x, opt.rect.right()));

    if (columnRoleList.isEmpty())
        return;
    int role = columnRoleList.at(0);

    if (index != d->editingIndex || (role != DFileSystemModel::FileNameRole && role != DFileSystemModel::FileDisplayNameRole)) {
        const QVariantHash &ep = index.data(DFileSystemModel::ExtraProperties).toHash();
        const QList<QColor> &colors = qvariant_cast<QList<QColor>>(ep.value("colored"));

        if (!colors.isEmpty()) {
            qreal diameter = 10;
            QRectF tag_boundingRect(0, 0, (colors.size() + 1) * diameter / 2, diameter);

            tag_boundingRect.moveCenter(rect.center());
            tag_boundingRect.moveRight(rect.right());

            paintCircleList(painter, tag_boundingRect, diameter, colors,
                            (drawBackground || colors.size() > 1) ? Qt::white : QColor(0, 0, 0, 25));
            rect.setRight(tag_boundingRect.left() - ICON_SPACING);
        }

        /// draw file name label
        const QVariant &data = index.data(role);
        painter->setPen(opt.palette.color(drawBackground ? QPalette::BrightText : QPalette::Text));
        if (data.canConvert<QString>()) {
            QString file_name;

            do {
                if (role != DFileSystemModel::FileNameRole && role != DFileSystemModel::FileDisplayNameRole) {
                    break;
                }

                if (role == DFileSystemModel::FileDisplayNameRole) {
                    const auto fileName = index.data(DFileSystemModel::FileNameRole);
                    const auto file_display_name = index.data(DFileSystemModel::FileDisplayNameRole);

                    if (fileName != file_display_name) {
                        break;
                    }
                }

                const QString &suffix = "." + index.data(DFileSystemModel::FileSuffixRole).toString();

                if (suffix == ".") {
                    break;
                }

                file_name = DFMGlobal::elideText(index.data(DFileSystemModel::FileBaseNameRole).toString().remove('\n'),
                                                 QSize(rect.width() - opt.fontMetrics.width(suffix), rect.height()), QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                 opt.font, Qt::ElideRight,
                                                 d->textLineHeight);
                file_name.append(suffix);
            } while (false);

            if (file_name.isEmpty()) {
                file_name = DFMGlobal::elideText(index.data(role).toString().remove('\n'),
                                                 rect.size(), QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                 opt.font, Qt::ElideRight,
                                                 d->textLineHeight);
            }

            painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), file_name);
        } else {
            drawNotStringData(opt, d->textLineHeight, rect, data, drawBackground, painter, 0);
        }
    }

    if (isDragMode) {
        return;
    }

    const DFileSystemModel *model = qobject_cast<const DFileSystemModel *>(index.model());
    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));
    else
        painter->setPen(opt.palette.color(QPalette::Inactive, QPalette::Text));

    for (int i = 1; i < columnRoleList.count(); ++i) {
        int column_width = parent()->columnWidth(i);

        if (column_width <= 0) {
            continue;
        }

        QRect rec = opt.rect;

        rec.setLeft(column_x + COLUMU_PADDING);

        if (rec.left() >= rec.right()) {
            break;
        }

        column_x += column_width;

        rec.setRight(qMin(column_x, opt.rect.right()));

        int rol = columnRoleList.at(i);

        QModelIndex tmp_index = model->createIndex(index.row(), model->roleToColumn(rol), index.internalId());

        const QVariant &data = index.data(rol);

        if (data.canConvert<QString>()) {
            QString strInfo(index.data(rol).toString());
            // 如果是文件路径项
            if(rol == DFileSystemModel::FilePathRole) {
                // 如果是保险箱路径,则不显示真实路径
                if(VaultController::isVaultFile(strInfo))
                    strInfo = VaultController::localPathToVirtualPath(index.data(rol).toString());
            }
            const QString &text = DFMGlobal::elideText(strInfo, rec.size(),
                                                       QTextOption::NoWrap, opt.font,
                                                       Qt::ElideRight, d->textLineHeight);

            painter->drawText(rec, Qt::Alignment(tmp_index.data(Qt::TextAlignmentRole).toInt()), text);
        } else {
            drawNotStringData(opt, d->textLineHeight, rec, data, drawBackground, painter, i);
        }
    }

    if (isDropTarget && !drawBackground) {
        QPen pen;
        QRectF rec = opt.rect;

        rec += QMarginsF(-0.5 + LEFT_PADDING, -0.5, -0.5 + RIGHT_PADDING, -0.5);

        pen.setColor(opt.backgroundBrush.color());

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(rec, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::black);
    }

    painter->setOpacity(1);
}


void DListItemDelegate::drawNotStringData(const QStyleOptionViewItem &opt, int lineHeight, const QRect &rect, const QVariant &data,
                                          bool drawBackground, QPainter *painter, const int &column) const
{

    const DFileSystemModel *model = parent()->model();
    const DAbstractFileInfoPointer &fileInfo = model->fileInfo(model->rootUrl());

    int sortRole = model->sortRole();
    int sortRoleIndexByColumnChildren = fileInfo->userColumnChildRoles(column).indexOf(sortRole);

    const QColor &active_color = opt.palette.color(QPalette::Active, drawBackground ? QPalette::BrightText : QPalette::Text);
    const QColor &normal_color = opt.palette.color(QPalette::Inactive, drawBackground ? QPalette::BrightText : QPalette::Text);

    if (data.canConvert<QPair<QString, QString>>()) {
        QPair<QString, QString> name_path = qvariant_cast<QPair<QString, QString>>(data);

        const QString &file_name = DFMGlobal::elideText(name_path.first.remove('\n'),
                                                        QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                        opt.font, Qt::ElideRight,
                                                        lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, 0, 0, -rect.height() / 2), Qt::AlignBottom, file_name);

        const QString &file_path = DFMGlobal::elideText(name_path.second.remove('\n'),
                                                        QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                        opt.font, Qt::ElideRight,
                                                        lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, rect.height() / 2, 0, 0), Qt::AlignTop, file_path);
    } else if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        QRect new_rect = rect;

        const QPair<QString, QPair<QString, QString>> &dst = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        const QString &date = DFMGlobal::elideText(dst.first, QSize(rect.width(), rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, 0, 0, -new_rect.height() / 2), Qt::AlignBottom, date, &new_rect);

        new_rect = QRect(rect.left(), rect.top(), new_rect.width(), rect.height());

        const QString &size = DFMGlobal::elideText(dst.second.first, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignLeft, size);

        const QString &type = DFMGlobal::elideText(dst.second.second, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideLeft, lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 2 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignRight, type);
    }
}


QSize DListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const DAbstractFileInfoPointer &file_info = parent()->fileInfo(index);

    if (!file_info) {
        return DFMStyledItemDelegate::sizeHint(option, index);
    }

    Q_D(const DListItemDelegate);

    return QSize(d->itemSizeHint.width(), qMax(file_info->userRowHeight(option.fontMetrics), d->itemSizeHint.height()));
}

QWidget *DListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_D(const DListItemDelegate);

    d->editingIndex = index;

    QLineEdit *edit = new QLineEdit(parent);

    const DAbstractFileInfoPointer &file_info = this->parent()->fileInfo(index);
    if (file_info->fileUrl().isSearchFile()) {
        edit->setFixedHeight(LIST_EDITER_HEIGHT * 2 - 10);
    } else {
        edit->setFixedHeight(LIST_EDITER_HEIGHT);
    }
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

        text = DFMGlobal::preprocessingFileName(text);

        QVector<uint> list = text.toUcs4();
        int cursor_pos = edit->cursorPosition() - text_length + text.length();

        const QString &suffix = edit->property("_d_whether_show_suffix").toString();

        while (text.toLocal8Bit().size() > MAX_FILE_NAME_CHAR_COUNT - suffix.size() - (suffix.isEmpty() ? 0 : 1))
        {
            list.removeAt(--cursor_pos);

            text = QString::fromUcs4(list.data(), list.size());
        }

        if (text.count() != old_text.count())
        {
            edit->setText(text);
            edit->setCursorPosition(cursor_pos);
        }
    });

    edit->setFrame(false);
    edit->setAttribute(Qt::WA_TranslucentBackground);
    edit->setContentsMargins(0, 0, 0, 0);

    return edit;
}

void DListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
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
    rect.setTop(opt_rect.y() + (opt_rect.height() - editor->height()) / 2);

    editor->setGeometry(rect);
}

void DListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool donot_show_suffix{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };
    QString text{};

    if (donot_show_suffix) {
        edit->setProperty("_d_whether_show_suffix", index.data(DFileSystemModel::FileSuffixOfRenameRole));
        text = index.data(DFileSystemModel::FileBaseNameOfRenameRole).toString();
    } else {
        text = index.data(DFileSystemModel::FileNameOfRenameRole).toString();
    }

    edit->setText(text);
}

static int dataWidth(const QStyleOptionViewItem &option, const QModelIndex &index, int role)
{
    const QVariant &data = index.data(role);
    Qt::Alignment alignment = Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt());

    if (data.canConvert<QString>()) {
        return option.fontMetrics.width(data.toString(), -1, alignment);
    }

    if (data.canConvert<QPair<QString, QString>>()) {
        const QPair<QString, QString> &string_string = qvariant_cast<QPair<QString, QString>>(data);

        return qMax(option.fontMetrics.width(string_string.first, -1, alignment), option.fontMetrics.width(string_string.second, -1, alignment));
    }

    if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        const QPair<QString, QPair<QString, QString>> &string_p_string = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        return option.fontMetrics.width(string_p_string.first, -1, alignment);
    }

    return -1;
}

// sizeHintMode为true时，计算列的宽度时计算的为此列真实需要的宽度，而不受实际列宽所限制
QList<QRect> DListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
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

    int role = columnRoleList.at(0);

    if (sizeHintMode) {
        rect.setWidth(dataWidth(option, index, role));
        column_x = rect.right();
    } else {
        column_x = parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

        rect.setRight(qMin(column_x, opt_rect.right()));
        /// draw file name label
        rect.setWidth(qMin(rect.width(), dataWidth(option, index, role)));
    }

    geomertys << rect;

    for (int i = 1; i < columnRoleList.count(); ++i) {
        QRect rec = opt_rect;

        rec.setLeft(column_x + COLUMU_PADDING);

        if (rec.left() >= opt_rect.right()) {
            return geomertys;
        }

        int rol = columnRoleList.at(i);

        if (sizeHintMode) {
            rec.setWidth(dataWidth(option, index, rol));
            column_x += rec.width();
        } else {
            column_x += parent()->columnWidth(i) - 1;

            rec.setRight(qMin(column_x, opt_rect.right()));
            rec.setWidth(qMin(rec.width(), dataWidth(option, index, rol)));
        }

        geomertys << rec;
    }

    return geomertys;
}

void DListItemDelegate::updateItemSizeHint()
{
    Q_D(DListItemDelegate);

    d->textLineHeight = parent()->parent()->fontMetrics().lineSpacing();
    d->itemSizeHint = QSize(-1, qMax(int(parent()->parent()->iconSize().height() * 1.1), d->textLineHeight));
}

bool DListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    Q_D(DListItemDelegate);

    if (event->type() == QEvent::Show) {
        QLineEdit *edit = qobject_cast<QLineEdit *>(object);

        if (edit) {
            const QString &selectionWhenEditing = parent()->baseName(d->editingIndex);
            int endPos = selectionWhenEditing.isEmpty() ? -1 : selectionWhenEditing.length();

            if (endPos == -1) {
                edit->selectAll();
            } else {
                edit->setSelection(0, endPos);
            }
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            QLineEdit *edit = qobject_cast<QLineEdit *>(object);

            if (edit) {
                edit->close();
                edit->parentWidget()->setFocus();
            }

            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

static void hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

bool DListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::ToolTipRole).toString();

        if (tooltip.isEmpty()) {
//            QToolTip::hideText();
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }

        return true;
    }

    return DFMStyledItemDelegate::helpEvent(event, view, option, index);
}
