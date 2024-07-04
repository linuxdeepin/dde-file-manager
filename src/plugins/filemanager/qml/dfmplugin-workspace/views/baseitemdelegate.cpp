// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseitemdelegate.h"
#include "private/baseitemdelegate_p.h"
#include "fileview.h"
#include "abstractitempaintproxy.h"
#include "utils/fileviewhelper.h"
#include "utils/itemdelegatehelper.h"
#include "events/workspaceeventcaller.h"
#include "models/fileviewmodel.h"

#include <dfm-base/dfm_base_global.h>

#include <QTextLayout>
#include <QPainter>

DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

BaseItemDelegate::BaseItemDelegate(FileViewHelper *parent)
    : BaseItemDelegate(*new BaseItemDelegatePrivate(this), parent)
{
}

BaseItemDelegate::BaseItemDelegate(BaseItemDelegatePrivate &dd, FileViewHelper *parent)
    : QStyledItemDelegate(parent),
      d(&dd)
{
    dd.init();
}

void BaseItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    parent()->initStyleOption(option, index);
}

QList<QRectF> BaseItemDelegate::getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const
{
    QList<QRectF> list;
    double offset = baseRect.width() / 8;
    const QSizeF &offsetSize = cornerSize / 2;

    list.append(QRectF(QPointF(baseRect.right() - offset - offsetSize.width(),
                               baseRect.bottom() - offset - offsetSize.height()),
                       cornerSize));
    list.append(QRectF(QPointF(baseRect.left() + offset - offsetSize.width(), list.first().top()), cornerSize));
    list.append(QRectF(QPointF(list.at(1).left(), baseRect.top() + offset - offsetSize.height()), cornerSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), cornerSize));

    return list;
}

void BaseItemDelegate::paintEmblems(QPainter *painter, const QRectF &iconRect, const QModelIndex &index) const
{
    const FileInfoPointer &info = parent()->parent()->model()->fileInfo(index);
    if (info)
        WorkspaceEventCaller::sendPaintEmblems(painter, iconRect, info);
}

bool BaseItemDelegate::isThumnailIconIndex(const QModelIndex &index) const
{
    if (!index.isValid() || !parent())
        return false;

    FileInfoPointer info { parent()->fileInfo(index) };
    if (info) {
        const auto &attribute { info->extendAttributes(ExtInfoType::kFileThumbnail) };
        if (attribute.isValid() && !attribute.value<QIcon>().isNull())
            return true;
    }
    return false;
}

BaseItemDelegate::~BaseItemDelegate()
{
}

QSize BaseItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    Q_D(const BaseItemDelegate);

    return d->itemSizeHint;
}

void BaseItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::destroyEditor(editor, index);

    d->editingIndex = QModelIndex();
}

int BaseItemDelegate::iconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::minimumIconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::maximumIconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::increaseIcon()
{
    return -1;
}

int BaseItemDelegate::decreaseIcon()
{
    return -1;
}

int BaseItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_UNUSED(level)
    return -1;
}

QModelIndexList BaseItemDelegate::hasWidgetIndexs() const
{
    Q_D(const BaseItemDelegate);

    if (!d->editingIndex.isValid())
        return QModelIndexList();
    return QModelIndexList() << d->editingIndex;
}

void BaseItemDelegate::hideAllIIndexWidget()
{
    hideNotEditingIndexWidget();

    if (d->editingIndex.isValid()) {
        parent()->parent()->setIndexWidget(d->editingIndex, nullptr);

        d->editingIndex = QModelIndex();
    }
}

void BaseItemDelegate::hideNotEditingIndexWidget()
{
}

void BaseItemDelegate::commitDataAndCloseActiveEditor()
{
    QWidget *editor = parent()->indexWidget(d->editingIndex);

    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget *, editor));
}

QRectF BaseItemDelegate::itemIconRect(const QRectF &itemRect) const
{
    return QRectF(itemRect);
}

QList<QRect> BaseItemDelegate::itemGeomertys(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_UNUSED(opt);
    Q_UNUSED(index);
    return {};
}

QRect BaseItemDelegate::getRectOfItem(RectOfItemType type, const QModelIndex &index) const
{
    Q_UNUSED(type)
    Q_UNUSED(index)

    return QRect();
}

bool BaseItemDelegate::itemExpanded()
{
    return false;
}

QRect BaseItemDelegate::expandItemRect()
{
    return QRect();
}

QModelIndex BaseItemDelegate::expandedIndex()
{
    return QModelIndex();
}

FileViewHelper *BaseItemDelegate::parent() const
{
    return dynamic_cast<FileViewHelper *>(QStyledItemDelegate::parent());
}

void BaseItemDelegate::paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF iconRect = opt.rect;
    iconRect.setSize(size);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    ItemDelegateHelper::paintIcon(painter, opt.icon,
                                  { iconRect, Qt::AlignCenter,
                                    QIcon::Normal, QIcon::Off,
                                    ViewMode::kIconMode, isThumnailIconIndex(index) });
}

QSize BaseItemDelegate::getIndexIconSize(const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF iconRect = opt.rect;
    iconRect.setSize(size);

    QSize iconSize = opt.icon.actualSize(iconRect.size().toSize(), QIcon::Normal, QIcon::Off);
    if (iconSize.width() > size.width() || iconSize.height() > size.height())
        iconSize.scale(size, Qt::KeepAspectRatio);

    return iconSize;
}

void BaseItemDelegate::setPaintProxy(AbstractItemPaintProxy *proxy)
{
    if (d->paintProxy) {
        delete d->paintProxy;
        d->paintProxy = nullptr;
    }

    d->paintProxy = proxy;
}

QModelIndex BaseItemDelegate::editingIndex() const
{
    Q_D(const BaseItemDelegate);

    return d->editingIndex;
}

QWidget *BaseItemDelegate::editingIndexWidget() const
{
    Q_D(const BaseItemDelegate);

    return parent()->indexWidget(d->editingIndex);
}
