/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmsidebaritemdelegate.h"

#include "interfaces/dfmsidebaritem.h"
/*********************************************************/
//bug 26937 书签目标目录不存在情况下还可以重命名问题。相关头文件添加
#include "interfaces/dfileservices.h"
#include "dfmsidebarview.h"
#include "models/dfmsidebarmodel.h"
/*********************************************************/
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DPalette>
#include <qdrawutil.h>
#include <QLineEdit>

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMSideBarItemDelegate::DFMSideBarItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

void DFMSideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMSideBarItem::Separator) {
        return paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DFMSideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMSideBarItem::Separator) {
        return sizeHintForType(DFMSideBarItem::Separator);
    } else {
        return DStyledItemDelegate::sizeHint(option, index);
    }
}

void DFMSideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(model);
    QByteArray n = editor->metaObject()->userProperty().name();
    if (!n.isEmpty()) {
        emit rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *DFMSideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /***************************************************************************************************************/
    //此部分判断主要是为了解决书签目标目录不存在情况下仍可以对书签进行重命名并且重命名后可打开失效目录问题
    //目前在标签对应目标路径不存在的情况下，右键重命名是失效的，所以也不应允许双击修改。故此处做规避
    //有用过QFileInfo::exists直接path路径有问题所以改成了DAbstractFileInfoPointer
    DFMSideBarView *sidebarView = dynamic_cast<DFMSideBarView*>(this->parent());
    DFMSideBarModel *sidebarModel = dynamic_cast<DFMSideBarModel*>(sidebarView->model());
    DFMSideBarItem *tgItem = sidebarModel->itemFromIndex(index);

    const DAbstractFileInfoPointer &sourceInfo = DFileService::instance()->createFileInfo(nullptr, tgItem->url());
    if(!sourceInfo->exists())
        return nullptr;
    /***************************************************************************************************************/
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        QRegExp regx("^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*"); //屏蔽特殊字符
        QValidator *validator = new QRegExpValidator(regx, qle);
        qle->setValidator(validator);
        const QString &fs = sourceInfo->extraProperties()["fsType"].toString();
        // 普通文件系统限制最长输入字符为 40, vfat exfat 由于文件系统的原因，只能输入 11 个字符
        int maxLenInBytes = fs.toLower().endsWith("fat") ? 11 : 40;
        connect(qle, &QLineEdit::textChanged, this, [qle, maxLenInBytes](const QString &txt) {
            if (!qle)
                return;
            if (txt.toUtf8().length() > maxLenInBytes) {
                const QSignalBlocker blocker(qle);
                QString newLabel = txt;
                newLabel.chop(1);
                qle->setText(newLabel);
            }
        });
    }

    return editor;
}

void DFMSideBarItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    DStyledItemDelegate::updateEditorGeometry(editor, option, index);

    //DTK在计算editor宽度的时候没有考虑icon的宽度，导致editor超出view的范围，超出部分看不到了，这里需要调整editor的宽度。
    DFMSideBarView *sidebarView = dynamic_cast<DFMSideBarView*>(this->parent());
    editor->setFixedWidth(sidebarView->width() - 59);
}

void DFMSideBarItemDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}

QSize DFMSideBarItemDelegate::sizeHintForType(int type) const
{
    switch (type) {
    case DFMSideBarItem::Separator:
        return QSize(200, 5);
    }

    return QSize(0, 0);
}

DFM_END_NAMESPACE
