// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebaritemdelegate.h"

#include "views/sidebaritem.h"
#include "models/sidebarmodel.h"
#include "views/sidebarview.h"

#include "dfm-base/base/schemefactory.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DPalette>
#include <qdrawutil.h>
#include <QLineEdit>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace GlobalPrivate {
void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option)
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}
}   // namespace GlobalPrivate

SideBarItemDelegate::SideBarItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void SideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    if (dynamic_cast<SideBarItemSeparator *>(item)) {
        return GlobalPrivate::paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    if (dynamic_cast<SideBarItemSeparator *>(item)) {
        return QSize(200, 5);
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void SideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(model);
    QByteArray n = editor->metaObject()->userProperty().name();
    if (!n.isEmpty()) {
        Q_EMIT rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *SideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /***************************************************************************************************************/
    //此部分判断主要是为了解决书签目标目录不存在情况下仍可以对书签进行重命名并且重命名后可打开失效目录问题
    //目前在标签对应目标路径不存在的情况下，右键重命名是失效的，所以也不应允许双击修改。故此处做规避
    //有用过QFileInfo::exists直接path路径有问题所以改成了DAbstractFileInfoPointer
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(sidebarView->model());
    SideBarItem *tgItem = sidebarModel->itemFromIndex(index);

    auto sourceInfo = InfoFactory::create<AbstractFileInfo>(tgItem->url());
    if (!sourceInfo)
        return nullptr;
    if (!sourceInfo->exists())
        return nullptr;
    /***************************************************************************************************************/
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        QRegExp regx("^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*");   //屏蔽特殊字符
        QValidator *validator = new QRegExpValidator(regx, qle);
        qle->setValidator(validator);
        const QString &fs = sourceInfo->extraProperties()["fsType"].toString();
        // 普通文件系统限制最长输入字符为 40, vfat exfat 由于文件系统的原因，只能输入 11 个字符
        int maxLen = fs.toLower().endsWith("fat") ? 11 : 40;
        qle->setMaxLength(maxLen);
    }

    return editor;
}

void SideBarItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    DStyledItemDelegate::updateEditorGeometry(editor, option, index);

    //DTK在计算editor宽度的时候没有考虑icon的宽度，导致editor超出view的范围，超出部分看不到了，这里需要调整editor的宽度。
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    editor->setFixedWidth(sidebarView->width() - 59);
}
