// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/delegatecommon.h"
#include "iconitemdelegate.h"
#include "private/iconitemdelegate_p.h"

#include <QMouseEvent>
#include "utils/itemdelegatehelper.h"
#include "utils/fileviewhelper.h"
#include "fileview.h"
#include "iconitemeditor.h"
#include "models/fileviewmodel.h"
#include "events/workspaceeventsequence.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>

#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DStyleOption>
#include <DStyle>
#include <DApplication>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QToolTip>
#include <QtMath>

#include <linux/limits.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

IconItemDelegate::IconItemDelegate(FileViewHelper *parent)
    : BaseItemDelegate(*new IconItemDelegatePrivate(this), parent)
{
    Q_D(IconItemDelegate);

    fmDebug() << "Creating IconItemDelegate";

    d->expandedItem = new ExpandedItem(this, parent->parent()->viewport());
    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->expandedItem->setCanDeferredDelete(false);
    d->expandedItem->setContentsMargins(0, 0, 0, 0);
    /// prevent flash when first call show()
    d->expandedItem->setFixedWidth(0);

    fmDebug() << "Created expanded item widget";

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [d]() {
        if (d->expandedIndex.isValid() && !d->expandedItem->isHidden())
            d->expandedItem->repaint();
    });
#endif
    connect(parent, &FileViewHelper::triggerEdit, this, &IconItemDelegate::onTriggerEdit);

    d->itemIconSize = iconSizeByIconSizeLevel();
    parent->parent()->setIconSize(d->itemIconSize);

    fmDebug() << "IconItemDelegate initialization completed - icon size:" << d->itemIconSize;
}

IconItemDelegate::~IconItemDelegate()
{
    Q_D(IconItemDelegate);

    fmDebug() << "Destroying IconItemDelegate";

    if (d->expandedItem) {
        fmDebug() << "Cleaning up expanded item widget";
        d->expandedItem->setParent(nullptr);
        d->expandedItem->setCanDeferredDelete(true);
        d->expandedItem->deleteLater();
    }

    fmDebug() << "IconItemDelegate destruction completed";
}

void IconItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    // Check if this is a group header item
    if (isGroupHeaderItem(index)) {
        paintGroupHeader(painter, option, index);
        return;
    }

    Q_D(const IconItemDelegate);

    if (index == d->expandedIndex && !parent()->isSelected(index))
        const_cast<IconItemDelegate *>(this)->hideNotEditingIndexWidget();

    painter->setOpacity(parent()->isTransparent(index) ? 0.3 : 1.0);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);
    static QFont oldFont = opt.font;

    if (oldFont != opt.font) {
        if (d->expandedItem)
            d->expandedItem->setFont(opt.font);

        QWidget *editingWidget = editingIndexWidget();

        if (editingWidget)
            editingWidget->setFont(opt.font);

        const_cast<IconItemDelegate *>(this)->updateItemSizeHint();
    }

    oldFont = opt.font;

    // for debug
    // painter->drawRect(opt.rect);
    // if (index.row() == 0) {
    //     painter->drawRect(parent()->parent()->viewport()->rect());
    // }

    const QPainterPath &path = paintItemBackgroundAndGeomerty(painter, opt, index, 0);

    const QRectF &iconRect = paintItemIcon(painter, opt, index);

    paintItemFileName(painter, iconRect, path, 0, opt, index);

    painter->setOpacity(1);
}

bool IconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(kItemFileIconModelToolTipRole).toString();
        const QList<QRect> &geometries = paintGeomertys(option, index);

        bool hideTips = geometries.count() < 3;
        if (!hideTips) {
            int displayTextWidth = 0;
            for (int i = 1; i < geometries.count() - 1; ++i) {
                displayTextWidth += geometries[i].width();
            }
            displayTextWidth += 1;   // 误差
            hideTips = option.fontMetrics.horizontalAdvance(tooltip) <= displayTextWidth;
        }
        if (tooltip.isEmpty() || index == view->rootIndex() || hideTips) {   // 当从一个需要显示tooltip的icon上移动光标到不需要显示的icon上时立即隐藏当前tooltip
            ItemDelegateHelper::hideTooltipImmediately();
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

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QList<QRect> IconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    Q_UNUSED(sizeHintMode)
    Q_D(const IconItemDelegate);

    QList<QRect> geometries;

    if (index == d->expandedIndex) {
        QRect geometry = d->expandedItem->iconGeometry().toRect();
        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());

        geometries << geometry;

        geometry = d->expandedItem->textGeometry().toRect();
        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());
        geometry.setTop(geometries.first().bottom());

        geometries << geometry;

        return geometries;
    }

    // init icon geomerty
    QRect iconRect = option.rect;

    iconRect.setSize(parent()->parent()->iconSize());
    iconRect.moveCenter(option.rect.center());
    iconRect.moveTop(option.rect.top());

    auto itemIconRectf = itemIconRect(option.rect);
    geometries << itemIconRectf.toRect();

    const QString &fileName = displayFileName(index);

    if (fileName.isEmpty()) {
        return geometries;
    }

    // init file name geometry
    QRect labelRect = option.rect;
    labelRect.setTop(static_cast<int>(iconRect.bottom()) + kIconModeTextPadding + kIconModeIconSpacing);
    labelRect.setLeft(labelRect.left() + kIconModeRectRadius);
    labelRect.setWidth(labelRect.width() - kIconModeRectRadius);

    QStyleOptionViewItem opt = option;

    // if has selected show all file name else show elide file name.
    bool isSelected = parent()->isSelected(index) && opt.showDecorationSelected;
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    bool elide = (!isSelected || !singleSelected);

    QList<QRectF> lines = calFileNameRect(fileName, labelRect, elide ? opt.textElideMode : Qt::ElideNone);

    for (const QRectF &line : lines) {
        geometries << line.toRect();
    }

    // background rect
    QRectF backgroundRect = itemIconRectf;
    backgroundRect.adjust(-kIconModeIconSpacing, -kIconModeIconSpacing,
                          kIconModeIconSpacing, kIconModeIconSpacing);
    geometries << backgroundRect.toRect();

    return geometries;
}

void IconItemDelegate::updateItemSizeHint()
{
    Q_D(IconItemDelegate);

    int width = parent()->parent()->iconSize().width();
    if (d->viewDefines.indexOfIconSize(width) >= 0)
        width += kIconModeIconSpacing * 2;

    int customWidth = d->viewDefines.iconGridDensity(d->currentIconGridWidthIndex);
    if (customWidth > width)
        width = customWidth;

    // do not include 2 lines text height
    int height = parent()->parent()->iconSize().height()
            + kIconModeTextPadding   // 文字与icon之间的空隙
            + 2 * kIconModeIconSpacing;   // icon与背景的上下两个间距

    d->itemSizeHint = QSize(width, height);
}

int IconItemDelegate::iconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    return d->currentIconSizeIndex;
}

int IconItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int IconItemDelegate::maximumIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    return d->viewDefines.iconSizeCount() - 1;
}

int IconItemDelegate::increaseIcon()
{
    Q_D(const IconItemDelegate);

    int newLevel = setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
    return newLevel;
}

int IconItemDelegate::decreaseIcon()
{
    Q_D(const IconItemDelegate);

    int newLevel = setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
    return newLevel;
}

int IconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(IconItemDelegate);

    fmDebug() << "Setting icon size by level:" << level << "current level:" << iconSizeLevel();

    if (level == iconSizeLevel()) {
        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());
        fmDebug() << "Icon size level unchanged, refreshing view";
        return level;
    }

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;
        d->itemIconSize = iconSizeByIconSizeLevel();
        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());

        fmInfo() << "Icon size changed to level" << d->currentIconSizeIndex << "size:" << d->itemIconSize;
        return d->currentIconSizeIndex;
    }

    fmWarning() << "Invalid icon size level:" << level << "valid range:" << minimumIconSizeLevel() << "to" << maximumIconSizeLevel();
    return d->currentIconSizeIndex;
}

int IconItemDelegate::minimumWidthLevel() const
{
    Q_D(const IconItemDelegate);

    return d->currentIconGridWidthIndex;
}

void IconItemDelegate::setItemMinimumWidthByWidthLevel(int level)
{
    Q_D(IconItemDelegate);

    fmDebug() << "Setting item minimum width by level:" << level << "current level:" << d->currentIconGridWidthIndex;

    if (level >= 0 && level < d->viewDefines.iconGridDensityCount()) {
        int oldLevel = d->currentIconGridWidthIndex;
        d->currentIconGridWidthIndex = level;
        updateItemSizeHint();

        fmInfo() << "Item width level changed from" << oldLevel << "to" << level;
    } else {
        fmWarning() << "Invalid width level:" << level << "valid range: 0 to" << (d->viewDefines.iconGridDensityCount() - 1);
    }
}

void IconItemDelegate::hideNotEditingIndexWidget()
{
    Q_D(IconItemDelegate);

    if (d->expandedIndex.isValid()) {
        parent()->parent()->setIndexWidget(d->expandedIndex, nullptr);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedIndex = QModelIndex();
    }
}

QRectF IconItemDelegate::itemIconRect(const QRectF &itemRect) const
{
    // init icon geomerty
    QRectF iconRect = itemRect;
    iconRect.setSize(parent()->parent()->iconSize());
    iconRect.moveLeft(itemRect.left() + (itemRect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(iconRect.top() + kIconModeIconSpacing);
    return iconRect;
}

QList<QRect> IconItemDelegate::itemGeomertys(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QList<QRect> geometries;
    // init icon geomerty
    auto iconRect = itemIconRect(opt.rect);
    // background rect
    QRectF backgroundRect = iconRect;
    backgroundRect.adjust(-kIconModeIconSpacing, -kIconModeIconSpacing,
                          kIconModeIconSpacing, kIconModeIconSpacing);
    geometries << backgroundRect.toRect();

    const QString &fileName = displayFileName(index);

    if (fileName.isEmpty()) {
        return geometries;
    }

    // init file name geometry
    QRect labelRect = opt.rect;
    labelRect.setTop(static_cast<int>(iconRect.bottom()) + kIconModeTextPadding + kIconModeIconSpacing);
    labelRect.setLeft(labelRect.left() + kIconModeRectRadius);
    labelRect.setWidth(labelRect.width() - kIconModeRectRadius);

    QList<QRectF> lines = calFileNameRect(fileName, labelRect, opt.textElideMode);
    for (const auto &line : lines) {
        geometries.append(line.toRect());
    }
    return geometries;
}

bool IconItemDelegate::itemExpanded()
{
    Q_D(IconItemDelegate);

    return d->expandedIndex.isValid() && d->expandedItem->isVisible();
}

QRect IconItemDelegate::expandItemRect()
{
    Q_D(IconItemDelegate);

    if (d->expandedIndex.isValid() && d->expandedItem->isVisible()) {
        QRect itemRect = d->expandedItem->getOption().rect;
        QRect textRect = d->expandedItem->textGeometry().toRect();
        textRect.moveTopLeft(itemRect.topLeft() + textRect.topLeft());
        return textRect;
    }

    return QRect();
}

QModelIndex IconItemDelegate::expandedIndex()
{
    Q_D(IconItemDelegate);

    return d->expandedIndex;
}

QWidget *IconItemDelegate::expandedItem()
{
    Q_D(IconItemDelegate);
    return d->expandedItem;
}

QString IconItemDelegate::displayFileName(const QModelIndex &index) const
{
    bool showSuffix { Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool() };
    QString str = index.data(kItemFileDisplayNameRole).toString();
    const QString &suffix = "." + index.data(kItemFileSuffixRole).toString();

    if (!showSuffix && str.endsWith(suffix) && suffix != "." && suffix != str)
        str = str.mid(0, str.length() - suffix.length());

    return str;
}

QList<QRectF> IconItemDelegate::calFileNameRect(const QString &name, const QRectF &rect, Qt::TextElideMode elideMode) const
{
    int lineHeight = UniversalUtils::getTextLineHeight(name, parent()->parent()->fontMetrics());
    QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(name, QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                lineHeight, Qt::AlignCenter));
    return layout->layout(rect, elideMode);
}

void IconItemDelegate::editorFinished()
{
    FileViewHelper *viewHelper = parent();
    if (!viewHelper) {
        fmWarning() << "Cannot process editor finished: view helper is null";
        return;
    }

    FileView *fileview = viewHelper->parent();
    if (!fileview) {
        fmWarning() << "Cannot process editor finished: file view is null";
        return;
    }

    auto windowId = WorkspaceHelper::instance()->windowId(fileview);
    if (!fileview->model()) {
        fmWarning() << "Cannot process editor finished: file view model is null";
        return;
    }

    QUrl url = fileview->model()->data(d->editingIndex, kItemUrlRole).toUrl();
    WorkspaceEventCaller::sendRenameEndEdit(windowId, url);
}

void IconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(IconItemDelegate);

    if (index == d->expandedIndex) {
        parent()->parent()->setIndexWidget(index, nullptr);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedIndex = QModelIndex();
        parent()->parent()->edit(index);
    }
}

QPainterPath IconItemDelegate::paintItemBackgroundAndGeomerty(QPainter *painter, const QStyleOptionViewItem &option,
                                                              const QModelIndex &index, int backgroundMargin) const
{
    Q_UNUSED(index);
    Q_UNUSED(backgroundMargin);
    painter->save();

    bool isPaintForAnim = option.state & QStyle::State_AutoRaise;
    bool isDragMode = (static_cast<QPaintDevice *>(parent()->parent()->viewport()) != painter->device()) && !isPaintForAnim;
    bool isSelected = !isDragMode && (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    DPalette pl(DPaletteHelper::instance()->palette(option.widget));
    QColor backgroundColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);

    QColor baseColor = backgroundColor;
    bool isUpshow = false;
    if (option.widget) {
        baseColor = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
            isUpshow = true;
        }
    }

    bool isHover = option.state & QStyle::StateFlag::State_MouseOver;
    if (isDropTarget && !isSelected) {
        backgroundColor.setAlpha(40);   // DropTarg背景设置透明度为15% (40/255);
    } else if (option.state & QStyle::StateFlag::State_Selected) {
        backgroundColor.setAlpha(backgroundColor.alpha() + 40);
    } else if (isHover) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType && !isUpshow) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
            backgroundColor = baseColor;
        } else {
            backgroundColor = backgroundColor.lighter();
            backgroundColor.setAlpha(22);   // Hover背景设置透明度为8% (22/255);
        }
    } else {
        backgroundColor = baseColor;
    }

    QRectF backgroundRect = option.rect;
    QSizeF iconSize = parent()->parent()->iconSize();
    // 左右上下和icon的边距都是6
    backgroundRect.setSize(iconSize + QSizeF(2 * kIconModeIconSpacing, 2 * kIconModeIconSpacing));
    // for checkmark
    qreal backgroundx = (option.rect.width() - backgroundRect.width()) / 2.0;
    backgroundRect.moveLeft(backgroundRect.left() + backgroundx);   // x坐标居中
    // draw background
    QPainterPath path;
    QRectF outLineRect = backgroundRect;
    outLineRect.setSize(outLineRect.size() - QSizeF(2, 2));
    outLineRect.moveCenter(backgroundRect.center());
    path.addRoundedRect(outLineRect, kIconModeBackRadius, kIconModeBackRadius);

    if (isDropTarget || isSelected || isHover) {   // 只有选中和mouseover才绘制背景
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, backgroundColor);
        if (isHover) {
            backgroundColor.setAlpha(40);   // Hover背景边框设置透明度为16% (40/255);
            painter->setPen(backgroundColor);
            painter->drawPath(path);
        }
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    painter->restore();

    return path;
}

QRectF IconItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    // init icon geomerty
    QRectF iconRect = itemIconRect(opt.rect);

    bool isDropTarget = parent()->isDropTarget(index);
    if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(iconRect.size().toSize());
        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), iconRect.size().toSize()), QColor(0, 0, 0, (static_cast<int>(std::ceil(255 * 0.1)))));
        p.end();
        painter->drawPixmap(iconRect.toRect(), pixmap);
    } else {
        bool isEnabled = opt.state & QStyle::State_Enabled;
        // draw icon
        ItemDelegateHelper::paintIcon(painter, opt.icon,
                                      { iconRect,
                                        Qt::AlignCenter,
                                        isEnabled ? QIcon::Normal : QIcon::Disabled,
                                        QIcon::Off,
                                        ViewMode::kIconMode,
                                        isThumnailIconIndex(index) });
    }

    paintEmblems(painter, iconRect, index);

    return iconRect;
}

void IconItemDelegate::paintItemFileName(QPainter *painter, QRectF iconRect, QPainterPath path,
                                         int backgroundMargin, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_UNUSED(path);
    Q_UNUSED(backgroundMargin);
    Q_D(const IconItemDelegate);

    bool isPaintForAnim = opt.state & QStyle::State_AutoRaise;
    if (index == d->editingIndex && !isPaintForAnim)
        return;

    bool isDragMode = (static_cast<QPaintDevice *>(parent()->parent()->viewport()) != painter->device()) && !isPaintForAnim;

    if (index == d->expandedIndex && !isDragMode && !isPaintForAnim) {
        if (d->expandedItem && d->expandedItem->getIndex() == index
            && d->expandedItem->getOption().rect == opt.rect) {
            d->expandedItem->setOption(opt);
        }
        return;
    }

    // init file name geometry
    QRectF labelRect = opt.rect;
    labelRect.setTop(static_cast<int>(iconRect.bottom()) + kIconModeTextPadding + kIconModeIconSpacing);

    bool singleSelected = parent()->parent()->selectedIndexCount() < 2;
    bool isSelectedOpt = opt.state & QStyle::State_Selected;
    // 文管窗口拖拽时的字体保持白色
    if (isDragMode || (!singleSelected && isSelectedOpt)) {
        painter->setPen(opt.palette.color(QPalette::BrightText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Text));
    }

    const QString &displayName = displayFileName(index);

    // if has selected show all file name else show elide file name.
    bool isSelected = !isDragMode && isSelectedOpt && opt.showDecorationSelected;
    if (!singleSelected)
        d->expandedItem->setDifferenceOfLastRow(0);

    if (isSelected && singleSelected && !isPaintForAnim) {
        const_cast<IconItemDelegate *>(this)->hideNotEditingIndexWidget();
        /// init file name text
        d->expandedIndex = index;

        setEditorData(d->expandedItem, index);
        parent()->parent()->setIndexWidget(index, d->expandedItem);

        // 重设item状态
        d->expandedItem->setIndex(index);
        d->expandedItem->setOption(opt);
        d->expandedItem->setTextBounding(QRectF());
        d->expandedItem->setFixedWidth(0);
        d->expandedItem->setDifferenceOfLastRow(parent()->parent()->rowCount() - parent()->parent()->indexOfRow(index) - 1);

        updateEditorGeometry(d->expandedItem, opt, index);

        return;
    } else {
        if (!singleSelected) {
            const_cast<IconItemDelegate *>(this)->hideNotEditingIndexWidget();
            d->lastAndExpandedIndex = QModelIndex();
        }
    }

    // 图标拖拽时保持活动色
    auto background = isDragMode || (!singleSelected && isSelectedOpt)
            ? (opt.palette.brush(QPalette::Normal, QPalette::Highlight))
            : QBrush(Qt::NoBrush);
    int lineHeight = UniversalUtils::getTextLineHeight(displayName, parent()->parent()->fontMetrics());
    QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(displayName, QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                lineHeight, Qt::AlignCenter, painter));
    layout->setHighlightEnabled(!isSelected);
    layout->setHighlightKeywords(parent()->parent()->model()->getKeyWords());
    layout->setHighlightColor(QColor("#0081FF"));

    labelRect.setLeft(labelRect.left() + kIconModeRectRadius);
    labelRect.setWidth(labelRect.width() - kIconModeRectRadius);
    const FileInfoPointer &info = parent()->fileInfo(index);
    WorkspaceEventSequence::instance()->doIconItemLayoutText(info, layout.data());
    if (!singleSelected && isSelectedOpt) {
        layout->setAttribute(ElideTextLayout::kBackgroundRadius, kIconModeRectRadius);
    }

    // If the filename is very long, sizeHint() will set the height of the last item to maximum
    // to make the scrollbar appear on the right side.
    // However, when the last item is not a single selection target,
    // we don't need to draw the filename at maximum height
    if (!isSelected || !singleSelected) {
        qreal normalHeight = lineHeight * 2;
        labelRect.setHeight(labelRect.height() > normalHeight ? normalHeight : labelRect.height());
    }

    QStringList textList {};
    layout->layout(labelRect, opt.textElideMode, painter, background, &textList);
}

QSize IconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    int size = d->viewDefines.iconSize(d->currentIconSizeIndex);

    return QSize(size, size);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Check if this is a group header item
    if (isGroupHeaderItem(index)) {
        return getGroupHeaderSizeHint(option, index);
    }

    Q_D(const IconItemDelegate);

    QSize size = d->itemSizeHint;

    // Note: calculate text line height by file display name
    // can not use fontMetrics height() to calculate text line height, because the fontMetrics is not accurate
    int lineHeight = UniversalUtils::getTextLineHeight(index, parent()->parent()->fontMetrics());
    size.setHeight(size.height() + 2 * lineHeight);

    // If there is one selection and the name is very long,
    // set the item height to maximum for the last index to make the scrollbar appear on the right side
    if (index.isValid() && parent()->isLastIndex(index) && d->expandedItem
        && d->expandedIndex.isValid() && d->expandedItem->isVisible()) {
        d->expandedItem->setIconHeight(parent()->parent()->iconSize().height());
        auto hight = qMax(size.height(), d->expandedItem->heightForWidth(size.width()) - d->expandedItem->getDifferenceOfLastRow() * size.height());
        return QSize(size.width(), hight);
    }

    return size;
}

QWidget *IconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    Q_D(const IconItemDelegate);

    d->editingIndex = index;

    IconItemEditor *editor = new IconItemEditor(parent);

    connect(editor, &IconItemEditor::inputFocusOut,
            this, &IconItemDelegate::commitDataAndCloseActiveEditor,
            Qt::UniqueConnection);

    connect(editor, &IconItemEditor::inputFocusOut, this, &IconItemDelegate::editorFinished);

    connect(editor, &IconItemEditor::destroyed, this, [this, d] {
        QWidget *editor = this->parent()->indexWidget(d->editingIndex);
        if (!editor || editor == sender()) {
            d->editingIndex = QModelIndex();
        }
    });

    // 设置字体居中
    // NOTE: 此处经过查阅发现FileItem中editUndo中没有相关设置
    // 触发撤销将不会产生字体对其,这里稍微提及,方便以后更改
    editor->getTextEdit()->setAlignment(Qt::AlignHCenter);
    editor->getTextEdit()->document()->setTextWidth(d->itemSizeHint.width());
    editor->setOpacity(this->parent()->isTransparent(index) ? 0.3 : 1);

    if (FileViewHelper *viewHelper = this->parent()) {
        if (FileView *fileView = viewHelper->parent()) {
            if (fileView->model()) {
                auto windowId = WorkspaceHelper::instance()->windowId(parent);
                QUrl url = fileView->model()->data(index, kItemUrlRole).toUrl();
                WorkspaceEventCaller::sendRenameStartEdit(windowId, url);
            }
        }
    }

    return editor;
}

void IconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const IconItemDelegate);

    auto fileview = parent()->parent();
    if (fileview == nullptr)
        return;

    const QSize &iconSize = fileview->iconSize();

    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (editor == d->expandedItem) {
        // 重置textBounding，使其在adjustSize重新计算，否则在调整图标大小时使用旧的textBounding计算导致显示不全
        d->expandedItem->show();
        d->expandedItem->setTextBounding(QRect());
        editor->setFixedWidth(option.rect.width());
        d->expandedItem->setIconHeight(iconSize.height());
        editor->adjustSize();
        return;
    }

    editor->setFixedWidth(option.rect.width());

    IconItemEditor *item = qobject_cast<IconItemEditor *>(editor);

    if (!item)
        return;

    FileViewHelper *viewHelper = qobject_cast<FileViewHelper *>(parent());
    int maxHeight = viewHelper ? (viewHelper->viewContentSize().height() - viewHelper->verticalOffset() - item->pos().y()) : INT_MAX;
    if (viewHelper && fileview->size().height() > viewHelper->viewContentSize().height())
        maxHeight = fileview->size().height() - viewHelper->verticalOffset() - item->pos().y();

    item->setMaxHeight(qMax(maxHeight, sizeHint(opt, index).height()));

    QLabel *icon = item->getIconLabel();

    if (iconSize.height() != icon->size().height()) {
        double iconTopOffset = kIconModeIconSpacing;   // update edit pos
        icon->setFixedHeight(iconSize.height() + static_cast<int>(std::ceil(iconTopOffset)));
    }
}

void IconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyleOptionViewItem opt;

    initStyleOption(&opt, index);
    const QSize &iconSize = parent()->parent()->iconSize();

    if (ExpandedItem *item = qobject_cast<ExpandedItem *>(editor)) {
        item->setIconHeight(iconSize.height());
        item->setOpacity(parent()->isTransparent(index) ? 0.3 : 1);

        return;
    }

    IconItemEditor *item = qobject_cast<IconItemEditor *>(editor);

    if (!item) {
        fmWarning() << "Editor is not an IconItemEditor, cannot set data";
        return;
    }

    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();
    const QUrl &fileUrl = index.data(kItemUrlRole).toUrl();
    const QString &filename = index.data(kItemNameRole).toString();

    // if file is in dlnfs' path, use char count rather than byte count to limit the filename length
    if (FileUtils::supportLongName(fileUrl) || filename.toLocal8Bit().length() > NAME_MAX)
        item->setCharCountLimit();

    QString suffix = index.data(kItemFileSuffixOfRenameRole).toString();
    fmDebug() << "Display" << index.data(kItemFileDisplayNameRole).toString()
              << "FileName" << index.data(kItemNameRole).toString()
              << "FileNameofrenmae" << index.data(kItemFileNameOfRenameRole).toString()
              << "BaseName" << index.data(kItemFileBaseNameRole).toString()
              << "BaseNameofrename" << index.data(kItemFileBaseNameOfRenameRole).toString()
              << "suffix" << index.data(kItemFileSuffixRole).toString()
              << "suffixofrename" << suffix;

    if (showSuffix) {
        QString name = index.data(kItemFileNameOfRenameRole).toString();
        name = FileUtils::preprocessingFileName(name);

        item->setMaxCharSize(NAME_MAX);
        item->setText(name);
        item->select(name.left(name.size() - suffix.size() - (suffix.isEmpty() ? 0 : 1)));

        fmDebug() << "Set editor with suffix - full name:" << name << "suffix:" << suffix;
    } else {
        item->setProperty(kEidtorShowSuffix, suffix);
        item->setMaxCharSize(NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1));

        QString name = index.data(kItemFileBaseNameOfRenameRole).toString();
        name = FileUtils::preprocessingFileName(name);

        item->setText(name);
        item->select(name);

        fmDebug() << "Set editor without suffix - base name:" << name << "hidden suffix:" << suffix;
    }
}

bool IconItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

// Group functionality implementation
int IconItemDelegate::getGroupHeaderHeight(const QStyleOptionViewItem &option) const
{
    Q_UNUSED(option)

    // Fixed height of 32 pixels for icon mode group headers
    return 32;
}

bool IconItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Handle group header clicks
    if (isGroupHeaderItem(index) && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        handleGroupHeaderClick(mouseEvent, option, index);
        return true;
    }

    // Handle double-click on group headers for expand/collapse
    if (isGroupHeaderItem(index) && event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            const QString groupKey = index.data(Global::kItemGroupHeaderKey).toString();
            if (!groupKey.isEmpty()) {
                emit const_cast<IconItemDelegate *>(this)->groupExpansionToggled(groupKey);
            }
            return true;
        }
    }

    // Call base class implementation for regular items
    return BaseItemDelegate::editorEvent(event, model, option, index);
}

void IconItemDelegate::handleGroupHeaderClick(QMouseEvent *event, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !index.isValid()) {
        return;
    }

    // Extract group key from index
    QUrl url = index.data(Global::kItemUrlRole).toUrl();
    const QString groupKey = index.data(Global::kItemGroupHeaderKey).toString();
    if (groupKey.isEmpty()) {
        return;
    }

    // Check if click is on expand button
    QRect expandButtonRect = getExpandButtonRect(option);
    if (expandButtonRect.contains(event->pos())) {
        // Toggle expansion
        emit const_cast<IconItemDelegate *>(this)->groupExpansionToggled(groupKey);
    } else {
        // Click on group header text area
        emit const_cast<IconItemDelegate *>(this)->groupHeaderClicked(groupKey);
    }
}
