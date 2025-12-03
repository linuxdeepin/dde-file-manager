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
#include <dfm-base/utils/iconutils.h>

#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DStyleOption>

#include <QTextLayout>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
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
        if (IconUtils::shouldSkipThumbnailFrame(info->nameOf(NameInfoType::kMimeTypeName)))
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

    // Paint background
    paintGroupBackground(painter, option);

    painter->save();
    // Get group information
    QString groupText = index.data(Qt::DisplayRole).toString();
    int fileCount = index.data(Global::kItemGroupFileCount).toInt();
    if (groupText.isEmpty()) {
        groupText = "Group";
    }

    // Get expansion state - assume expanded by default for now
    bool isExpanded = index.data(Global::ItemRoles::kItemGroupExpandedRole).toBool();

    // Calculate layout rectangles
    QRect expandButtonRect = getExpandButtonRect(option);
    QRect textRect = getGroupTextRect(option);

    // Paint expand button
    paintExpandButton(painter, expandButtonRect, isExpanded);

    // Paint group text
    paintGroupText(painter, textRect, groupText, fileCount, option);

    painter->restore();
}

QRectF BaseItemDelegate::getGroupHeaderBackgroundRect(const QStyleOptionViewItem &option) const
{
    // Default implementation: no margins
    return option.rect;
}

void BaseItemDelegate::paintGroupBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    if (!painter || !option.widget) {
        return;
    }

    painter->save();
    DPalette pl(DPaletteHelper::instance()->palette(option.widget));
    QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor adjustColor = baseColor;

    // Choose background color based on state
    if (option.state & QStyle::State_MouseOver) {
        adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
    } else {
        painter->setOpacity(0);
    }

    // Get background rect from subclass (allows different implementations for different view modes)
    QRectF rect = getGroupHeaderBackgroundRect(option);

    // Use rounded rect path for smooth appearance (8px radius)
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);

    // Set render antialiasing for smooth rounded corners
    painter->setRenderHints(QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    painter->fillPath(path, adjustColor);
    painter->restore();
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

void BaseItemDelegate::paintGroupText(QPainter *painter, const QRect &textRect, const QString &text, int count, const QStyleOptionViewItem &option) const
{
    if (!painter || textRect.isEmpty() || text.isEmpty() || count < 0) {
        return;
    }

    painter->save();

    QString filecountText = count <= 1 ? QObject::tr("%1 item").arg(count)
                                       : QObject::tr("%1 items").arg(count);

    // Get fonts from DFontSizeManager
    QFont groupTitleFont = DFontSizeManager::instance()->t6();   // T6 for group title
    QFont fileCountFont = DFontSizeManager::instance()->t8();   // T8 for file count

    // Determine base text color based on theme
    bool isLightTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::LightType;

    // Set group title color: 70% black/white
    QColor groupTitleColor = isLightTheme ? QColor(0, 0, 0, qRound(255 * 0.7))
                                          : QColor(255, 255, 255, qRound(255 * 0.7));

    // Set file count color: 50% black/white
    QColor fileCountColor = isLightTheme ? QColor(0, 0, 0, qRound(255 * 0.5))
                                         : QColor(255, 255, 255, qRound(255 * 0.5));

    // Calculate text widths
    QFontMetrics groupTitleFm(groupTitleFont);
    QFontMetrics fileCountFm(fileCountFont);

    int fileCountWidth = fileCountFm.horizontalAdvance(filecountText);
    int spacing = 6;   // 6px spacing between title and count
    int availableWidthForTitle = textRect.width() - fileCountWidth - spacing;

    // Draw group title
    painter->setFont(groupTitleFont);
    painter->setPen(groupTitleColor);

    QString elidedTitle = groupTitleFm.elidedText(text, Qt::ElideRight, availableWidthForTitle);
    int titleWidth = groupTitleFm.horizontalAdvance(elidedTitle);

    QRect titleRect = textRect;
    titleRect.setWidth(titleWidth);
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, elidedTitle);

    // Draw file count
    painter->setFont(fileCountFont);
    painter->setPen(fileCountColor);

    QRect countRect = textRect;
    countRect.setLeft(titleRect.left() + titleWidth + spacing);
    painter->drawText(countRect, Qt::AlignLeft | Qt::AlignVCenter, filecountText);

    painter->restore();
}

QRect BaseItemDelegate::getExpandButtonRect(const QStyleOptionViewItem &option) const
{
    QRect buttonRect;
    buttonRect.setSize(m_expandButtonSize);
    buttonRect.moveLeft(option.rect.left() + 12);   // 12px left margin for expand button
    buttonRect.moveTop(option.rect.top() + (option.rect.height() - m_expandButtonSize.height()) / 2);

    return buttonRect;
}

QRect BaseItemDelegate::getGroupTextRect(const QStyleOptionViewItem &option) const
{
    QRect expandButtonRect = getExpandButtonRect(option);

    QRect textRect;
    textRect.setLeft(expandButtonRect.right() + 8);   // 8px spacing after button
    textRect.setTop(option.rect.top());
    textRect.setRight(option.rect.right() - 12);   // 12px right margin
    textRect.setBottom(option.rect.bottom());

    return textRect;
}
