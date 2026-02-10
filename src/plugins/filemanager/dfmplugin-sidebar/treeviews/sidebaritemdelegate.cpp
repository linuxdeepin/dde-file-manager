// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebaritemdelegate.h"

#include "sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include "sidebarview.h"
#include "events/sidebareventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/devicealiasmanager.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DDciIcon>
#include <DStyle>

#include <QLineEdit>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <qdrawutil.h>
#include <QImage>
#include <QFontMetrics>
#include <QEvent>
#include <QMouseEvent>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolTip>
#include <QPixmap>

#include <linux/limits.h>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static constexpr int kRadius = 8;
static constexpr int kExpandIconSize = 12;
static constexpr int kItemMargin = 10;
static constexpr int kItemIconSize = 16;
static constexpr int kEjectIconSize = 16;
static constexpr int kEmptyItemSize = 10;
static constexpr int kDefaultMaxLength = 40;

#ifdef DTKWIDGET_CLASS_DSizeMode
static constexpr int kCompactExpandIconSize = 10;
static constexpr int kCompactModeIcon = 16;
#endif

namespace GlobalPrivate {
const static char *const kRegPattern { "^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*" };
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
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);
    painter->save();

    QStyleOptionViewItem opt = option;

    DStyledItemDelegate::initStyleOption(&opt, index);
    if (opt.widget && opt.widget->isActiveWindow())
        opt.state |= QStyle::State_Active;
    else
        opt.state &= (~QStyle::State_Active);

    const bool isActive = opt.widget && opt.widget->isActiveWindow();

    painter->setRenderHint(QPainter::Antialiasing);

    if (!option.widget)
        return;

    DPalette palette(DPaletteHelper::instance()->palette(option.widget));
    auto widgetColor = option.widget->palette().base().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        widgetColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);

    DStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);

    if (!item)
        return DStyledItemDelegate::paint(painter, option, index);
    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
    // bug-205621
    QRect itemRect = qApp->devicePixelRatio() > 1.0 ? opt.rect.adjusted(0, 1, 0, -1) : opt.rect;
    QPoint dx = QPoint(kItemMargin, 0);
    QPoint dw = QPoint(-12, 0);
    bool selected = opt.state.testFlag(QStyle::State_Selected);
    QRect r(itemRect.topLeft() + dx, itemRect.bottomRight() + dw);
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());

    bool isDragedItem = sidebarView->isSideBarItemDragged();
    bool isDropTarget = sidebarView->isDropTarget(index);
    bool keepDrawingHighlighted = false;
    const auto &itemUrl = index.data(SideBarItem::kItemUrlRole).toUrl();
    bool isUrlEqual = UniversalUtils::urlEquals(itemUrl, sidebarView->currentUrl());
    SideBarItem *subItem = dynamic_cast<SideBarItem *>(item);
    if (!isUrlEqual && subItem) {
        bool foundByCb = subItem->itemInfo().findMeCb && subItem->itemInfo().findMeCb(subItem->url(), sidebarView->currentUrl());
        if (foundByCb || UniversalUtils::urlEquals(subItem->url(), sidebarView->currentUrl()))
            isUrlEqual = true;
    }
    bool isDraggingItemNotHighlighted = selected && !isUrlEqual;
    if (isUrlEqual) {
        // If the dragging and moving source item is not the current highlighted one,
        // the highlighted one must be keep its state.
        keepDrawingHighlighted = true;
    }

    // Draw the background color when dragging files, rather than when dragging an item
    if ((selected && isDragedItem) || keepDrawingHighlighted) {   // Draw selected background
        QColor bgColor;
        if (isDraggingItemNotHighlighted) {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                bgColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);
            else
                bgColor = QColor(230, 230, 230);
        } else {
            bgColor = option.palette.color(QPalette::Active, QPalette::Highlight);
            if (!isActive)
                bgColor.setAlpha(102);
        }

        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(r, kRadius, kRadius);
    } else if (!isDragedItem && (opt.state.testFlag(QStyle::State_MouseOver) || isDropTarget)) {   // Draw mouse over background
        if (item->sizeHint() != QSize(kEmptyItemSize, kEmptyItemSize))
            drawMouseHoverBackground(painter, palette, r, widgetColor);
        if (separatorItem)
            drawMouseHoverExpandButton(painter, r, separatorItem->isExpanded());
    }

    QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
            ? QPalette::Normal
            : QPalette::Disabled;

    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
        cg = QPalette::Inactive;

        // Draw item icon
#ifdef DTKWIDGET_CLASS_DSizeMode
    QSize iconSize(DSizeModeHelper::element(QSize(kCompactModeIcon, kCompactModeIcon), QSize(kItemIconSize, kItemIconSize)));
#else
    QSize iconSize(kItemIconSize, kItemIconSize);
#endif

    SideBarItem *sidebarItem { static_cast<SideBarItem *>(item) };
    bool isEjectable { false };
    if (sidebarItem) {
        ItemInfo info { sidebarItem->itemInfo() };
        isEjectable = info.isEjectable;
    }
    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (!isDraggingItemNotHighlighted && (selected || keepDrawingHighlighted))
        iconMode = QIcon::Selected;
    if (opt.features & QStyleOptionViewItem::HasDecoration)
        drawIcon(opt, painter, index, itemRect, isEjectable, iconSize, iconMode, cg, keepDrawingHighlighted);

    // Draw item text
    QSize ejectIconSize(kEjectIconSize, kEjectIconSize);
    QFontMetrics metricsLabel(option.widget->font());

    QColor separatorTextColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        separatorTextColor = QColor(255, 255, 255, 102);   // alpha 40%
    } else {
        separatorTextColor = QColor(0, 0, 0, 76);   // alpha 30%
    }
    painter->setPen(separatorItem ? separatorTextColor : qApp->palette().color(QPalette::ColorRole::Text));
    if (iconMode == QIcon::Selected) {
        QColor finalTextColor = opt.palette.color(QPalette::Active, QPalette::HighlightedText);
        if (!isActive)
            finalTextColor.setAlpha(102);
        painter->setPen(finalTextColor);
    } else if (!isActive) {
        const QColor c = opt.palette.color(cg, QPalette::Text);
        painter->setPen(c);
    }

    QString text = index.data().toString();
    qreal baseValue = itemRect.width() - iconSize.width() - 2 * kItemMargin;
    qreal min = baseValue - 2 * ejectIconSize.width() - 10;
    qreal max = baseValue - ejectIconSize.width() - 10;

    if (metricsLabel.horizontalAdvance(text) > (isEjectable ? min : max))
        text = QFontMetrics(option.widget->font()).elidedText(text, Qt::ElideRight, (isEjectable ? int(min) : int(max)));
    int rowHeight = itemRect.height();
    qreal txtDx = (separatorItem ? 21 : 46);
    qreal txtDy = (itemRect.height() - metricsLabel.lineSpacing()) / 2 - 1;
    painter->drawText(QRectF(itemRect.x() + txtDx, itemRect.y() + txtDy, itemRect.width(), rowHeight), Qt::AlignLeft, text);
    painter->restore();
}

bool SideBarItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data().toString();
        QFontMetrics metricsLabel(option.widget->font());
        QRect itemRect = option.rect;
        qreal baseValue = itemRect.width() - kItemIconSize - 2 * kItemMargin;
        qreal min = baseValue - 2 * kItemIconSize - 10;
        qreal max = baseValue - kItemIconSize - 10;
        DStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
        SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
        bool isEjectable = false;
        if (sidebarItem) {
            ItemInfo info = sidebarItem->itemInfo();
            isEjectable = info.isEjectable;
        }
        if (metricsLabel.horizontalAdvance(tooltip) < (isEjectable ? min : max)) {
            QToolTip::hideText();
            return true;
        }
        QToolTip::showText(event->globalPos(), tooltip, view);
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

void SideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edt = nullptr;
    if ((edt = dynamic_cast<QLineEdit *>(editor)) && edt->isModified()) {
        QByteArray n = editor->metaObject()->userProperty().name();
        if (!n.isEmpty())
            Q_EMIT rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *SideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!this->parent())
        return nullptr;
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    if (!sidebarView)
        return nullptr;
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(sidebarView->model());
    if (!sidebarModel)
        return nullptr;
    SideBarItem *tgItem = sidebarModel->itemFromIndex(index);
    if (!tgItem)
        return nullptr;
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    // 重新设置调色板颜色值，使得背景色正确渲染，而不是保持透明状态。
    auto p = editor->palette();
    p.setColor(QPalette::Button, p.color(QPalette::Button));
    editor->setPalette(p);

    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        if (!NPDeviceAliasManager::instance()->canSetAlias(tgItem->targetUrl())) {
            QRegularExpression regx(GlobalPrivate::kRegPattern);
            QValidator *validator = new QRegularExpressionValidator(regx, qle);
            qle->setValidator(validator);
        }

        connect(qle, &QLineEdit::textChanged, this, [this, tgItem](const QString &text) {
            onEditorTextChanged(text, tgItem);
        });
    }

    return editor;
}

void SideBarItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto currEditor = qobject_cast<QLineEdit *>(editor);
    if (currEditor && index.isValid()) {
        DStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
        SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
        if (sidebarItem) {
            const auto &info = sidebarItem->itemInfo();
            currEditor->setText(info.editDisplayText.isEmpty() ? info.displayName : info.editDisplayText);
        }
    }
}

void SideBarItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    DStyledItemDelegate::updateEditorGeometry(editor, option, index);
    // When DTK calculates the width of editor, it does not care about the icon width, so adjust the width of editor here.
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    editor->setFixedWidth(sidebarView->width() - 50);
    QRect rect = editor->geometry();
    rect.setHeight(rect.height() + 2);
    rect.moveTo(40, rect.top());
    editor->setGeometry(rect);
}

bool SideBarItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid()) {
        QStandardItem *item = qobject_cast<const SideBarModel *>(model)->itemFromIndex(index);
        SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
        SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());

        if (event->type() == QEvent::MouseMove && separatorItem && sidebarView)
            sidebarView->update(index);
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            if (e->button() == Qt::LeftButton) {
                SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
                bool ejectable = false;
                if (sidebarItem) {
                    ItemInfo info = sidebarItem->itemInfo();
                    ejectable = info.isEjectable;
                }
                QRect expandBtRect(option.rect.width() - 40, option.rect.topRight().y() + 10, 24, 24);
                QRect ejectBtRect(option.rect.bottomRight() + QPoint(-28, -26), option.rect.bottomRight() + QPoint(-kItemMargin, -kItemMargin));
                QPoint pos = e->pos();
                if (event->type() != QEvent::MouseButtonRelease && separatorItem && expandBtRect.contains(pos)) {   // The expand/unexpand icon is pressed.
                    if (sidebarView)
                        Q_EMIT changeExpandState(index, !sidebarView->isExpanded(index));

                    event->accept();
                    return true;
                } else if (event->type() == QEvent::MouseButtonRelease && ejectable && ejectBtRect.contains(pos)) {   // The eject icon is pressed.
                    if (sidebarItem) {
                        QUrl url = sidebarItem->itemInfo().url;
                        SideBarEventCaller::sendEject(url);
                        // onItemActived() slot function would be triggered with mouse clicking,
                        // in order to avoid mount device again, we set item action to disable state as a mark.
                        DViewItemActionList list = sidebarItem->actionList(Qt::RightEdge);
                        if (list.count() > 0 && sidebarView) {
                            list.first()->setDisabled(true);
                            // fix bug: #185137, save the current url and highlight it in `SideBarWidget::onItemActived`
                            // that is triggered by cliking eject icon.
                            // this is the temporary solution.
                            list.first()->setProperty("currentItem", sidebarView->currentUrl());
                        }
                    }
                    event->accept();
                    return true;
                }
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void SideBarItemDelegate::onEditorTextChanged(const QString &text, SideBarItem *item) const
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    if (!editor)
        return;

    int maxLen = kDefaultMaxLength;
    bool useCharCount = false;

    auto info = InfoFactory::create<FileInfo>(item->url(), Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info && info->exists()) {
        const QString &fs = info->extraProperties()[GlobalServerDefines::DeviceProperty::kFileSystem].toString();
        if (fs.isEmpty()) {
            const auto &url = info->urlOf(FileInfo::FileUrlInfoType::kUrl);
            if (url.isLocalFile()) {
                maxLen = NAME_MAX;
                const auto &path = url.path();
                useCharCount = path.isEmpty() ? false : FileUtils::supportLongName(url);
            }
        } else {
            maxLen = FileUtils::supportedMaxLength(fs);
        }
    }

    QString dstText = text;
    int currPos = editor->cursorPosition();
    FileUtils::processLength(dstText, currPos, maxLen, useCharCount, dstText, currPos);

    if (text != dstText) {
        QSignalBlocker blocker(editor);
        editor->setText(dstText);
        editor->setCursorPosition(currPos);
        editor->setModified(true);
    }
}

void SideBarItemDelegate::drawIcon(const QStyleOptionViewItem &option,
                                   QPainter *painter,
                                   const QModelIndex &index,
                                   const QRect &itemRect,
                                   bool isEjectable,
                                   QSize iconSize,
                                   QIcon::Mode iconMode,
                                   QPalette::ColorGroup cg,
                                   bool keepHighlighted) const
{
    const bool isActive = option.widget && option.widget->isActiveWindow();
    if (iconMode == QIcon::Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    qreal iconDx = 2 * kItemMargin;
    qreal iconDy = (itemRect.height() - iconSize.height()) / 2;
    QPointF iconTopLeft = itemRect.topLeft() + QPointF(iconDx, iconDy);
    QRect iconRect(iconTopLeft.toPoint(), iconSize);

    painter->save();
    if (iconMode == QIcon::Selected && !isActive) {
        painter->setOpacity(0.4);
    }

    // 若为“按下未松开但未选中”的过渡态，属于非Selected状态
    bool pressedNotSelected = (option.state & QStyle::State_Sunken) && iconMode != QIcon::Selected;
    QStyleOptionViewItem optForIcon = option;
    if (pressedNotSelected)
        optForIcon.state &= ~QStyle::State_Selected;

    QVariant icon = index.data(Qt::DecorationRole);
    DDciIcon dciIcon;
    if (icon.canConvert<DTK_GUI_NAMESPACE::DDciIcon>())
        dciIcon = qvariant_cast<DDciIcon>(icon);
    if (dciIcon.isNull()) {
        QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
        option.icon.paint(painter, iconRect, option.decorationAlignment, iconMode, state);
    } else {
        drawDciIcon(optForIcon, painter, dciIcon, iconRect, cg, keepHighlighted);
    }
    
    painter->restore();

    // draw ejectable device icon
    if (isEjectable) {
        QIcon::Mode pixmapMode { (iconMode == QIcon::Selected) ? QIcon::Selected : QIcon::Normal };
        if (!option.widget->isActiveWindow()) {
            auto appTheme = DGuiApplicationHelper::toColorType(option.palette);
            if (appTheme == DGuiApplicationHelper::DarkType && pixmapMode == QIcon::Selected)
                pixmapMode = QIcon::Disabled;
            if (pixmapMode != QIcon::Selected)
                pixmapMode = QIcon::Disabled;
        }

        QSize ejectIconSize(kEjectIconSize, kEjectIconSize);
        QPoint ejectIconTopLeft = itemRect.bottomRight() + QPoint(0 - ejectIconSize.width() * 2, 0 - (itemRect.height() + ejectIconSize.height()) / 2);
        QPoint ejectIconBottomRight = ejectIconTopLeft + QPoint(ejectIconSize.width(), ejectIconSize.height());
        iconRect = QRect(ejectIconTopLeft, ejectIconBottomRight);
        DDciIcon dciIcon = DDciIcon::fromTheme("media-eject-symbolic");
        if (dciIcon.isNull()) {
            QIcon ejectIcon = QIcon::fromTheme("media-eject-symbolic");
            auto px { ejectIcon.pixmap(iconSize, pixmapMode, QIcon::On) };
            QStyle *style { option.widget ? option.widget->style() : QApplication::style() };
            style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, px);
        } else {
            drawDciIcon(option, painter, dciIcon, iconRect, cg, keepHighlighted);
        }
    }
}

void SideBarItemDelegate::drawDciIcon(const QStyleOptionViewItem &option, QPainter *painter,
                                      const DTK_GUI_NAMESPACE::DDciIcon &dciIcon, const QRect &iconRect,
                                      const QPalette::ColorGroup &cg, bool keepHighlighted) const
{
    DDciIcon::Mode mode = DStyle::toDciIconMode(&option);
    auto appTheme = DGuiApplicationHelper::toColorType(option.palette);
    DDciIcon::Theme theme = appTheme == DGuiApplicationHelper::LightType ? DDciIcon::Light : DDciIcon::Dark;
    DDciIconPalette palette { option.palette.color(cg, QPalette::WindowText), option.palette.color(cg, QPalette::Window),
                              option.palette.color(cg, QPalette::Highlight), option.palette.color(cg, QPalette::HighlightedText) };
    if ((option.state & QStyle::State_Selected) || keepHighlighted) {
        const QColor fg = painter->pen().color();
        palette.setForeground(fg);
    }
    dciIcon.paint(painter, iconRect, painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio(),
                  theme, mode, Qt::AlignCenter, palette);
}

void SideBarItemDelegate::drawMouseHoverBackground(QPainter *painter, const DPalette &palette, const QRect &r, const QColor &widgetColor) const
{
    QColor mouseHoverColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        mouseHoverColor = QColor(255, 255, 255, 25);   // 白色，10%透明度
    else
        mouseHoverColor = QColor(0, 0, 0, 25);   // 黑色，10%透明度

    painter->setBrush(mouseHoverColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(r, kRadius, kRadius);
}

void SideBarItemDelegate::drawMouseHoverExpandButton(QPainter *painter, const QRect &r, bool isExpanded) const
{
    painter->save();
    int iconSize = kExpandIconSize;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconSize = DSizeModeHelper::element(kCompactExpandIconSize, kExpandIconSize);
#endif

    int x = r.right() - 10 - iconSize;
    int y = r.top() + (r.height() / 2) - (iconSize / 2);
    QRect iconRect(QPoint(x, y), QSize(iconSize, iconSize));
    iconRect.moveTop(iconRect.top() - 1);

    bool isDarkType = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    QColor c(isDarkType ? qRgb(255, 255, 255) : qRgb(0, 0, 0));

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    QRect bgRect = iconRect.marginsAdded(QMargins(3, 3, 3, 3));
    if (bgRect.contains(sidebarView->mapFromGlobal(QCursor::pos()))) {
        painter->setOpacity(0.1);
        painter->drawRoundedRect(bgRect, kRadius, kRadius);
    }

    painter->setOpacity(1);
    painter->setPen(Qt::gray);
    QIcon icon = QIcon::fromTheme(isExpanded ? "go-up" : "go-down");
    icon.paint(painter, iconRect, Qt::AlignmentFlag::AlignCenter);
    painter->restore();
}
