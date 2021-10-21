/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "fileviewmodel.h"
#include "dfm_base_global.h"
#include "itemdelegatehelper.h"

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

DFMBASE_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE

ListItemDelegatePrivate::ListItemDelegatePrivate(ListItemDelegate *qq)
    : QObject (qq)
    , q(qq)
{

}

ListItemDelegate::ListItemDelegate(DListView *parent)
    : QStyledItemDelegate(parent)
    , d(new ListItemDelegatePrivate(this))
{
    d->listView = parent;
    parent->setIconSize(QSize(GlobalPrivate::LIST_VIEW_ICON_SIZE,
                              GlobalPrivate::LIST_VIEW_ICON_SIZE));
}

ListItemDelegate::~ListItemDelegate()
{

}

void ListItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{

    paintItemBackground(painter, option, index);

    QRect iconRect = paintItemIcon(painter, option, index);

    paintItemColumn(painter, option, index, iconRect);

    painter->setOpacity(1);
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *ListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    d->editingIndex = index;

    QLineEdit *edit = new QLineEdit(parent);

    const AbstractFileInfoPointer &file_info = qobject_cast<FileViewModel*>(this->parent())->fileInfo(index);
    if (file_info->url().scheme() == "search") {
        edit->setFixedHeight(GlobalPrivate::LIST_EDITER_HEIGHT * 2 - 10);
    } else {
        edit->setFixedHeight(GlobalPrivate::LIST_EDITER_HEIGHT);
    }
    edit->setObjectName("DListItemDelegate_Editor");

    connect(edit, &QLineEdit::destroyed, this, [=] {
        d->editingIndex = QModelIndex();
    });

    connect(edit, &QLineEdit::textChanged, this, [=] {
        //在此处处理的逻辑是因为默认QAbstractItemView的QLineEdit重命名会被SelectAll
        //const 防止被改变
        const QString srcText = edit->text();
        //得到处理之后的文件名称
        QString dstText = GlobalPrivate::replaceFileName(srcText);

        //超出长度将不再被支持输入获取当前
        bool donot_show_suffix= false;/*{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };*/

        //获取当前编辑框支持的最大文字长度
        int textMaxLen = INT_MAX;
        if (donot_show_suffix) {
            const QString &suffix = d->editingIndex.data(FileViewItem::ItemFileSuffixRole).toString();
            edit->setProperty("_d_whether_show_suffix", suffix);
            textMaxLen = NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1);
        } else {
            textMaxLen = NAME_MAX;
        }

        //如果存在非法字符且更改了当前的文本文件
        if (srcText != dstText) {
            int currPos = edit->cursorPosition();
            //气泡提示
            if (!this->parent()) {
                return;
            }

            auto view = this->parent();

            if(!view)
                return;

            auto showPoint = d->listView->mapToGlobal( QPoint( edit->pos().x() + edit->width() / 2,
                                                        edit->pos().y() + edit->height() * 2));
            //背板主题一致
            auto color = d->listView->palette().background().color();

            GlobalPrivate::showAlertMessage(showPoint,
                                            color,
                                            QObject::tr("\"\'/\\[]:|<>+=;,?* are not allowed"));

            currPos += dstText.length() - srcText.length();
            QSignalBlocker blocker(edit);
            edit->setText(dstText);
            edit->setCursorPosition(currPos);
        }

        int textCurrLen = dstText.toLocal8Bit().size();
        int textRangeOutLen = textCurrLen - textMaxLen;
        //最大输入框字符控制逻辑
        if (textRangeOutLen > 0) {
            // fix bug 69627
            QVector<uint> list = dstText.toUcs4();
            int cursor_pos = edit->cursorPosition();
            while (dstText.toLocal8Bit().size() > textMaxLen && cursor_pos > 0) {
                list.removeAt(--cursor_pos);
                dstText = QString::fromUcs4(list.data(), list.size());
            }
            QSignalBlocker blocker(edit);
            edit->setText(dstText);
            edit->setCursorPosition(cursor_pos);
        }
    });

    edit->setFrame(false);
    edit->setAttribute(Qt::WA_TranslucentBackground);
    edit->setContentsMargins(0, 0, 0, 0);

    return edit;
}

void ListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::updateEditorGeometry(editor,option,index);
}

void ListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //TODO(li):目前这块代码是不需要的，后面调整需要新添加这个逻辑
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool isNotShowSuffix = false;
    QString text;

    if (isNotShowSuffix) {
        edit->setProperty("_d_whether_show_suffix", index.data(FileViewItem::ItemFileSuffixRole));
        text = index.data(FileViewItem::ItemNameRole).toString();
    } else {
        text = index.data(FileViewItem::ItemNameRole).toString();
    }

    edit->setText(text);
}

bool ListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    return QStyledItemDelegate::eventFilter(object, event);
}


bool ListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
/*!
 * \brief paintItemBackground 绘制listviewitemd的交替绘制和选中时的高亮绘制
 *
 * \return void
 **/
void ListItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();//保存之前的绘制样式
    //反走样抗锯齿
    painter->setRenderHints(QPainter::Antialiasing
                           |QPainter::TextAntialiasing
                           |QPainter::SmoothPixmapTransform);
    //绘制新的背景交替
    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    //默认调整色保持背板颜色
    QColor adjustItemAlterColor = baseColor;//交替色
    QColor adjustHoverItemColor = baseColor;//hover色
    if (option.widget) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
            adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        }

        if (ct == DGuiApplicationHelper::LightType) {
            adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
            adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        }
    }

    DListView *pptr = qobject_cast<DListView*>(parent());
    if (!pptr)
        return;

    //获取item范围
    auto itemRect = pptr->visualRect(index);
    //左右间隔10 px UI设计要求 选中与交替渐变背景
    QRect dstRect(itemRect.x() + LIST_MODE_LEFT_MARGIN,
                  itemRect.y(),
                  itemRect.width() - (LIST_MODE_LEFT_MARGIN + LIST_MODE_RIGHT_MARGIN),
                  itemRect.height());

    //取模设置当前的交替变化
    if (index.row() % 2 == 1) {
        auto blockColorBrush = pptr->palette().background();
        QPainterPath path;
        path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);//圆角8 UI要求
        painter->fillPath(path, adjustItemAlterColor);
    } else {
        painter->setBrush(baseColor);
    }
    //设置hover高亮
    if (option.state & QStyle::StateFlag::State_MouseOver) {
        QPainterPath path;
        path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->fillPath(path, adjustHoverItemColor);
    }

    painter->restore(); //恢复之前的绘制，防止在此逻辑前的绘制丢失

    painter->save();//保存之前的绘制样式
    //反走样抗锯齿
    painter->setRenderHints(QPainter::Antialiasing
                            |QPainter::TextAntialiasing
                            |QPainter::SmoothPixmapTransform);
    //绘制新的背景交替
    if(option.widget){
        //调色板获取
        DPalette pl(DApplicationHelper::instance()->palette(option.widget));
        //背板颜色获取
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        //获取item范围
        auto itemRect = pptr->visualRect(index);
        //左右间隔10 px UI设计要求 选中与交替渐变背景
        QRect dstRect(itemRect.x() + LIST_MODE_LEFT_MARGIN,
                      itemRect.y(),
                      itemRect.width() - (LIST_MODE_LEFT_MARGIN + LIST_MODE_RIGHT_MARGIN),
                      itemRect.height());
        //取模设置当前的交替变化
        if (index.row() % 2 == 1) {
            //如果hover则设置高亮，不绘制交替色
            if (option.state & QStyle::StateFlag::State_MouseOver) {
                QColor adjustHoverItemColor = baseColor;//hover色 默认调整色保持背板颜色
                //hover色保持背板%10
                adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
                QPainterPath path;
                path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
                painter->fillPath(path, adjustHoverItemColor);
            } else { //绘制交替色
                QColor adjustItemAlterColor = baseColor;//交替色 默认调整色保持背板颜色
                //交替色保持背板色%5
                adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
                QPainterPath path;
                path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);//圆角8 UI要求
                painter->fillPath(path, adjustItemAlterColor);
            }
        } else {
            //如果hover则设置高亮，不保持默认背板
            if (option.state & QStyle::StateFlag::State_MouseOver) {//设置hover高亮
                QColor adjustHoverItemColor = baseColor;//hover色 默认调整色保持背板颜色
                adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
                QPainterPath path;
                path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
                //hover色保持背板%10
                painter->fillPath(path, adjustHoverItemColor);
            } else { //保持默认背板颜色
                painter->setBrush(baseColor);
            }
        }
    }
    painter->restore(); //恢复之前的绘制，防止在此逻辑前的绘制丢失
}
/*!
 * \brief paintItemIcon 绘制listviewitemd的icon
 *
 * \return QRect 返回绘制icon的区域方便后面绘制
 **/
QRect ListItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DListView *pptr = qobject_cast<DListView*>(parent());
    if (!pptr)
        return QRect();

    bool isEnabled = option.state & QStyle::State_Enabled;
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont oldFont = opt.font;
    oldFont = opt.font;
    opt.rect += QMargins(-LIST_MODE_LEFT_MARGIN, 0, -LIST_MODE_RIGHT_MARGIN, 0);

    // draw background
    bool drawBackground = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    QPalette::ColorGroup cg = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    QPalette::ColorRole colorRole = QPalette::Background;
    if ((isSelected)) {
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
    }

    opt.rect.setLeft(opt.rect.left() + LIST_MODE_LEFT_PADDING);
    opt.rect.setRight(opt.rect.right() - LIST_MODE_RIGHT_PADDING);

    // draw icon
    QRect iconRect = opt.rect;
    iconRect.setSize(pptr->iconSize());
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
    //绘制需要绘制的项，计算每一项绘制的宽度
    const QList<QPair<int,int>> &columnRoleList = index.data(FileViewItem::ItemColumListRole).value<QList<QPair<int,int>>>();
    if (columnRoleList.isEmpty())
        return;

    QStyleOptionViewItem opt = option;
    int column_x = iconRect.right() + LIST_MODE_ICON_SPACING;
    int columnWidtd = opt.rect.width() - column_x;

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));
    else
        painter->setPen(opt.palette.color(QPalette::Inactive, QPalette::Text));
    // 绘制那些需要显示的xiang
    for (int i = 0; i < columnRoleList.count(); ++i) {
        int column_width = columnWidtd * (columnRoleList.at(i).second <= 0 ? 0 : columnRoleList.at(i).second) / 100;
        if (column_width <= 0) {
            continue;
        }

        QRect columnRec = opt.rect;
        columnRec.setLeft(column_x + LIST_MODE_COLUMU_PADDING);

        if (columnRec.left() >= columnRec.right()) {
            break;
        }

        column_x += column_width;
        columnRec.setRight(qMin(column_x, opt.rect.right()));
        int rol = columnRoleList.at(i).first;
        const QVariant &data = index.data(rol);

        if (data.canConvert<QString>()) {
            QString strInfo(index.data(rol).toString());
            const QString &text = strInfo;
            painter->drawText(columnRec, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), text);
        }
    }
}
DFMBASE_END_NAMESPACE
