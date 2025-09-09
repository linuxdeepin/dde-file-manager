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
#include <QApplication>
#include <QStyle>

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
        // appimage 不显示缩略图底板
        if (info->nameOf(NameInfoType::kMimeTypeName) == Global::Mime::kTypeAppAppimage)
            return false;

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
    d->commitDataCurentWidget = nullptr;
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
// Repeated reentry of this function will cause a crash
void BaseItemDelegate::commitDataAndCloseActiveEditor()
{
    QWidget *editor = parent()->indexWidget(d->editingIndex);

    if (!editor)
        return;

    if (d->commitDataCurentWidget == editor)
        return;
    d->commitDataCurentWidget = editor;

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

QWidget *BaseItemDelegate::expandedItem()
{
    return nullptr;
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

// Group rendering implementation
bool BaseItemDelegate::isGroupHeaderItem(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    return index.data(Global::kItemIsGroupHeaderType).toBool();
}

QSize BaseItemDelegate::getGroupHeaderSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    int width = option.rect.width();
    int height = getGroupHeaderHeight(option);
    return QSize(width, height);
}

void BaseItemDelegate::paintGroupHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!painter || !index.isValid()) {
        return;
    }

    painter->save();

    // Paint background
    paintGroupBackground(painter, option);

    // Get group information
    QString groupText = index.data(Qt::DisplayRole).toString();
    if (groupText.isEmpty()) {
        groupText = "Group";
    }

    // Get expansion state - assume expanded by default for now
    // TODO: This should be retrieved from the model or worker
    bool isExpanded = true;

    // Calculate layout rectangles
    QRect expandButtonRect = getExpandButtonRect(option);
    QRect textRect = getGroupTextRect(option);

    // Paint expand button
    paintExpandButton(painter, expandButtonRect, isExpanded);

    // Paint group text
    paintGroupText(painter, textRect, groupText, option);

    painter->restore();
}

void BaseItemDelegate::paintGroupBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    if (!painter) {
        return;
    }

    // Choose background color based on state
    QColor backgroundColor;
    if (option.state & QStyle::State_MouseOver) {
        backgroundColor = option.palette.color(QPalette::AlternateBase);
    } else {
        backgroundColor = option.palette.color(QPalette::Base);
        // Make it slightly different from normal items
        backgroundColor = backgroundColor.lighter(105);
    }

    painter->fillRect(option.rect, backgroundColor);
}

void BaseItemDelegate::paintExpandButton(QPainter *painter, const QRect &buttonRect, bool isExpanded) const
{
    if (!painter || buttonRect.isEmpty()) {
        return;
    }

    painter->save();

    // Set up style option for drawing arrow
    QStyleOptionViewItem arrowOption;
    arrowOption.rect = buttonRect.marginsRemoved(QMargins(2, 2, 2, 2));
    arrowOption.palette = QApplication::palette();

    // Set pen color
    painter->setPen(arrowOption.palette.color(QPalette::Text));
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    // Draw appropriate arrow based on expansion state
    QStyle *style = QApplication::style();
    if (isExpanded) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOption, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &arrowOption, painter, nullptr);
    }

    painter->restore();
}

void BaseItemDelegate::paintGroupText(QPainter *painter, const QRect &textRect, const QString &text, const QStyleOptionViewItem &option) const
{
    if (!painter || textRect.isEmpty() || text.isEmpty()) {
        return;
    }

    painter->save();

    // Set up font - make it slightly bold
    QFont font = option.font;
    font.setBold(true);
    painter->setFont(font);

    // Set text color
    QColor textColor;
    if (option.state & QStyle::State_Selected) {
        textColor = option.palette.color(QPalette::HighlightedText);
    } else {
        textColor = option.palette.color(QPalette::Text);
    }
    painter->setPen(textColor);

    // Draw text with elision if necessary
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(text, Qt::ElideRight, textRect.width());

    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

    painter->restore();
}

QRect BaseItemDelegate::getExpandButtonRect(const QStyleOptionViewItem &option) const
{
    QRect buttonRect;
    buttonRect.setSize(m_expandButtonSize);
    buttonRect.moveLeft(option.rect.left() + m_leftMargin);
    buttonRect.moveTop(option.rect.top() + (option.rect.height() - m_expandButtonSize.height()) / 2);

    return buttonRect;
}

QRect BaseItemDelegate::getGroupTextRect(const QStyleOptionViewItem &option) const
{
    QRect expandButtonRect = getExpandButtonRect(option);

    QRect textRect;
    textRect.setLeft(expandButtonRect.right() + 8);   // 8px spacing after button
    textRect.setTop(option.rect.top());
    textRect.setRight(option.rect.right() - m_rightMargin);
    textRect.setBottom(option.rect.bottom());

    return textRect;
}
