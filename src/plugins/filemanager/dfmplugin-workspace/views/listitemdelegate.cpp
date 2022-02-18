/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "private/delegatecommon.h"
#include "private/listitemdelegate_p.h"
#include "listitemdelegate.h"
#include "fileview.h"
#include "fileviewitem.h"
#include "models/fileviewmodel.h"
#include "dfm-base/dfm_base_global.h"
#include "utils/itemdelegatehelper.h"
#include "utils/fileviewhelper.h"

#include "dfm-base/base/application/application.h"

#include <DListView>
#include <DArrowRectangle>
#include <DPalette>
#include <DApplicationHelper>

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainterPath>

#include <linux/limits.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

ListItemDelegate::ListItemDelegate(FileViewHelper *parent)
    : BaseItemDelegate(*new ListItemDelegatePrivate(this), parent)
{
    parent->parent()->setIconSize(QSize(GlobalPrivate::kListViewIconSize,
                                        GlobalPrivate::kListViewIconSize));
}

ListItemDelegate::~ListItemDelegate()
{
}

void ListItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);
    static QFont oldFont = option.font;

    if (oldFont != option.font) {
        QWidget *editingWidget = editingIndexWidget();

        if (editingWidget) {
            editingWidget->setFont(option.font);
        }

        const_cast<ListItemDelegate *>(this)->updateItemSizeHint();
    }

    oldFont = option.font;

    paintItemBackground(painter, opt, index);

    QRect iconRect = paintItemIcon(painter, opt, index);

    paintItemColumn(painter, opt, index, iconRect);

    painter->setOpacity(1);
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const AbstractFileInfoPointer &fileInfo = parent()->fileInfo(index);

    if (!fileInfo) {
        return BaseItemDelegate::sizeHint(option, index);
    }

    Q_D(const ListItemDelegate);

    // Todo(yanghao): isColumnCompact (fontMetrics.height() * 2 + 10)
    return QSize(d->itemSizeHint.width(), qMax(option.fontMetrics.height(), d->itemSizeHint.height()));
}

QWidget *ListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    D_DC(ListItemDelegate);
    Q_UNUSED(option);

    d->editingIndex = index;
    d->editor = new QLineEdit(parent);

    const AbstractFileInfoPointer &fileInfo = this->parent()->fileInfo(index);

    if (fileInfo->url().scheme() == "search") {
        d->editor->setFixedHeight(GlobalPrivate::kListEditorHeight * 2 - 10);
    } else {
        d->editor->setFixedHeight(GlobalPrivate::kListEditorHeight);
    }
    d->editor->setObjectName("DListItemDelegate_Editor");

    connect(d->editor, &QLineEdit::destroyed, this, [=] {
        d->editingIndex = QModelIndex();
        d->editor = nullptr;
    });
    connect(d->editor, &QLineEdit::textChanged, this, &ListItemDelegate::onEditorTextChanged);

    d->editor->setFrame(false);
    d->editor->setAttribute(Qt::WA_TranslucentBackground);
    d->editor->setContentsMargins(0, 0, 0, 0);

    return d->editor;
}

void ListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    const QSize &iconSize = parent()->parent()->iconSize();
    int columnX = 0;

    const QRect &optRect = option.rect + QMargins(-kListModeLeftMargin - kListModeLeftPadding, 0, -kListModeRightMargin - kListModeRightMargin, 0);
    QRect iconRect = optRect;
    iconRect.setSize(iconSize);

    const QList<QPair<int, int>> &columnRoleList = index.data(FileViewItem::kItemColumListRole).value<QList<QPair<int, int>>>();
    if (columnRoleList.isEmpty())
        return;
    QRect rect = optRect;
    for (int i = 0; i < columnRoleList.length(); ++i) {
        int rol = columnRoleList.at(i).first;
        if (rol == FileViewItem::kItemNameRole) {
            int iconOffset = i == 0 ? iconRect.right() + 1 : 0;

            rect.setLeft(columnX + iconOffset);
            columnX += parent()->parent()->getColumnWidth(i) - 1 - parent()->fileViewViewportMargins().left();

            rect.setRight(qMin(columnX, optRect.right()));
            rect.setTop(optRect.y() + (optRect.height() - editor->height()) / 2);
            break;
        } else {
            columnX += parent()->parent()->getColumnWidth(i);
        }
    }

    editor->setGeometry(rect);
}

void ListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // TODO(li):目前这块代码是不需要的，后面调整需要新添加这个逻辑
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool isNotShowSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();
    QString text = index.data(FileViewItem::kItemFileNameOfRenameRole).toString();

    if (isNotShowSuffix) {
        edit->setProperty("_d_whether_show_suffix", index.data(FileViewItem::kItemFileSuffixRole));
        text = index.data(FileViewItem::kItemFileNameOfRenameRole).toString();
    }

    edit->setText(text);
}

bool ListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
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

bool ListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(FileViewItem::kItemToolTipRole).toString();

        if (tooltip.isEmpty()) {
            ItemDelegateHelper::hideTooltipImmediately();
        } else {
            int tooltipSize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipSize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
            // Todo(yanghao): fix 81894
        }
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QList<QRect> ListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    Q_UNUSED(sizeHintMode)

    return QList<QRect>();
}

void ListItemDelegate::updateItemSizeHint()
{
    Q_D(ListItemDelegate);

    d->textLineHeight = parent()->parent()->fontMetrics().lineSpacing();
    d->itemSizeHint = QSize(-1, qMax(int(parent()->parent()->iconSize().height() * 1.1), d->textLineHeight));
}

void ListItemDelegate::onEditorTextChanged(const QString &text)
{
    if (!d->editor)
        return;

    // 得到处理之后的文件名称
    QString dstText = GlobalPrivate::replaceFileName(text);

    // 超出长度将不再被支持输入获取当前
    bool isNotShowSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();

    // 获取当前编辑框支持的最大文字长度
    int textMaxLen = INT_MAX;
    if (isNotShowSuffix) {
        const QString &suffix = d->editingIndex.data(FileViewItem::kItemFileSuffixRole).toString();
        d->editor->setProperty("_d_whether_show_suffix", suffix);
        textMaxLen = NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1);
    } else {
        textMaxLen = NAME_MAX;
    }

    // 如果存在非法字符且更改了当前的文本文件
    if (text != dstText) {
        int currPos = d->editor->cursorPosition();
        // 气泡提示
        if (!this->parent()) {
            return;
        }

        auto view = this->parent()->parent();

        if (!view)
            return;

        auto showPoint = view->mapToGlobal(QPoint(d->editor->pos().x() + d->editor->width() / 2,
                                                  d->editor->pos().y() + d->editor->height() * 2));
        // 背板主题一致
        auto color = view->palette().background().color();

        GlobalPrivate::showAlertMessage(showPoint,
                                        color,
                                        QObject::tr("\"\'/\\[]:|<>+=;,?* are not allowed"));

        currPos += dstText.length() - text.length();
        QSignalBlocker blocker(d->editor);
        d->editor->setText(dstText);
        d->editor->setCursorPosition(currPos);
    }

    int textCurrLen = dstText.toLocal8Bit().size();
    int textRangeOutLen = textCurrLen - textMaxLen;
    // 最大输入框字符控制逻辑
    if (textRangeOutLen > 0) {
        // fix bug 69627
        QVector<uint> list = dstText.toUcs4();
        int cursor_pos = d->editor->cursorPosition();
        while (dstText.toLocal8Bit().size() > textMaxLen && cursor_pos > 0) {
            list.removeAt(--cursor_pos);
            dstText = QString::fromUcs4(list.data(), list.size());
        }
        QSignalBlocker blocker(d->editor);
        d->editor->setText(dstText);
        d->editor->setCursorPosition(cursor_pos);
    }
}
/*!
 * \brief paintItemBackground 绘制listviewitemd的交替绘制和选中时的高亮绘制
 *
 * \return void
 **/
void ListItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();   // 保存之前的绘制样式
    // 反走样抗锯齿
    painter->setRenderHints(QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    FileView *view = parent()->parent();
    if (!view)
        return;

    // 获取item范围
    auto itemRect = view->visualRect(index);
    int totalWidth = view->getHeaderViewWidth();
    // 左右间隔10 px UI设计要求 选中与交替渐变背景
    QRect dstRect(itemRect.x() + kListModeLeftMargin,
                  itemRect.y(),
                  totalWidth - (kListModeLeftMargin + kListModeRightMargin),
                  itemRect.height());

    // draw background
    if (option.widget) {
        // 调色板获取
        DPalette pl(DApplicationHelper::instance()->palette(option.widget));
        // 背板颜色获取
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);

        // 取模设置当前的交替变化
        if (index.row() % 2 == 1) {
            // 如果hover则设置高亮，不绘制交替色
            if (option.state & QStyle::StateFlag::State_MouseOver) {
                // hover色保持背板%10
                QColor adjustHoverItemColor = baseColor;
                adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
                QPainterPath path;
                path.addRoundedRect(dstRect, kListModeRectRadius, kListModeRectRadius);
                painter->fillPath(path, adjustHoverItemColor);
            } else {   // 绘制交替色
                // 交替色保持背板色%5
                QColor adjustItemAlterColor = baseColor;
                adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
                QPainterPath path;
                path.addRoundedRect(dstRect, kListModeRectRadius, kListModeRectRadius);   //圆角8 UI要求
                painter->fillPath(path, adjustItemAlterColor);
            }
        } else {
            // 如果hover则设置高亮，不保持默认背板
            if (option.state & QStyle::StateFlag::State_MouseOver) {   //设置hover高亮
                // hover色 默认调整色保持背板颜色
                QColor adjustHoverItemColor = baseColor;
                adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
                QPainterPath path;
                path.addRoundedRect(dstRect, kListModeRectRadius, kListModeRectRadius);
                // hover色保持背板%10
                painter->fillPath(path, adjustHoverItemColor);
            } else {   // 保持默认背板颜色
                painter->setBrush(baseColor);
            }
        }
    }
    QPalette::ColorGroup cg = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
            ? QPalette::Normal
            : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    QPalette::ColorRole colorRole = QPalette::Background;
    if (isSelected) {
        colorRole = QPalette::Highlight;
        QPainterPath path;
        path.addRoundedRect(dstRect, kListModeRectRadius, kListModeRectRadius);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->restore();
    }
    painter->restore();   // 恢复之前的绘制，防止在此逻辑前的绘制丢失
}
/*!
 * \brief paintItemIcon 绘制listviewitemd的icon
 *
 * \return QRect 返回绘制icon的区域方便后面绘制
 **/
QRect ListItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!parent() || !parent()->parent())
        return QRect();

    bool isEnabled = option.state & QStyle::State_Enabled;
    QStyleOptionViewItem opt = option;

    opt.rect += QMargins(-kListModeLeftMargin, 0, -kListModeRightMargin, 0);

    opt.rect.setLeft(opt.rect.left() + kListModeLeftPadding);
    opt.rect.setRight(opt.rect.right() - kListModeRightPadding);

    // draw icon
    QRect iconRect = opt.rect;
    iconRect.setSize(parent()->parent()->iconSize());
    iconRect.moveTop(iconRect.top() + (opt.rect.bottom() - iconRect.bottom()) / 2);

    ItemDelegateHelper::paintIcon(painter, opt.icon, iconRect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);

    return iconRect;
}
/*!
 * \brief paintItemIcon 绘制listviewitemd的剩余每一列的数据
 *
 * \return void
 **/
void ListItemDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &iconRect) const
{
    D_DC(ListItemDelegate);
    // 绘制需要绘制的项，计算每一项绘制的宽度
    const QList<QPair<int, int>> &columnRoleList = index.data(FileViewItem::kItemColumListRole).value<QList<QPair<int, int>>>();
    if (columnRoleList.isEmpty())
        return;

    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);

    int column_x = iconRect.right() + kListModeIconSpacing;

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));

    // 绘制那些需要显示的项
    for (int i = 0; i < columnRoleList.count(); ++i) {
        int columnWidth = parent()->parent()->getColumnWidth(i);
        //第一列需要去掉图标宽度
        if (i == 0)
            columnWidth -= iconRect.right() + kListModeIconSpacing;

        if (columnWidth <= 0) {
            continue;
        }

        QRect columnRect = opt.rect;
        columnRect.setLeft(column_x);

        if (columnRect.left() >= columnRect.right()) {
            break;
        }

        column_x += columnWidth;
        columnRect.setRight(qMin(column_x - kListModeRightPadding, opt.rect.right() - kListModeRightPadding));
        int rol = columnRoleList.at(i).first;
        const QVariant &data = index.data(rol);

        QPalette::ColorGroup cGroup = QPalette::Inactive;
        Qt::TextElideMode elideMode = Qt::ElideRight;
        if (rol == FileViewItem::kItemNameRole) {
            cGroup = QPalette::Active;
            elideMode = Qt::ElideMiddle;
        }

        if (!isSelected)
            painter->setPen(opt.palette.color(cGroup, QPalette::Text));

        if (data.canConvert<QString>()) {
            QString displayString = ItemDelegateHelper::elideText(index.data(rol).toString().remove('\n'), columnRect.size(),
                                                                  QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                  option.font, elideMode,
                                                                  d->textLineHeight);

            painter->drawText(columnRect, index.data(Qt::TextAlignmentRole).toInt(), displayString);
        }
    }
}
