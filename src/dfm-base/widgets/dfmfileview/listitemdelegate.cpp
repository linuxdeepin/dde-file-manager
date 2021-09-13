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
    QStyledItemDelegate::paint(painter, option, index);
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
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool donot_show_suffix = false;
    QString text;

    if (donot_show_suffix) {
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
DFMBASE_END_NAMESPACE
