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
#include "iconitemdelegate.h"
#include "private/iconitemdelegate_p.h"
#include "utils/itemdelegatehelper.h"
#include "utils/fileviewhelper.h"
#include "fileview.h"
#include "iconitemeditor.h"
#include "models/filesortfilterproxymodel.h"
#include "events/workspaceeventsequence.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/application/application.h"

#include <DApplicationHelper>
#include <DStyleOption>
#include <DStyle>
#include <DApplication>

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

    d->expandedItem = new ExpandedItem(this, parent->parent()->viewport());
    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->expandedItem->setCanDeferredDelete(false);
    d->expandedItem->setContentsMargins(0, 0, 0, 0);
    /// prevent flash when first call show()
    d->expandedItem->setFixedWidth(0);

    connect(parent, &FileViewHelper::triggerEdit, this, &IconItemDelegate::onTriggerEdit);

    d->itemIconSize = iconSizeByIconSizeLevel();
    parent->parent()->setIconSize(d->itemIconSize);
}

IconItemDelegate::~IconItemDelegate()
{
    Q_D(IconItemDelegate);

    if (d->expandedItem) {
        d->expandedItem->setParent(nullptr);
        d->expandedItem->setCanDeferredDelete(true);
        d->expandedItem->deleteLater();
    }
}

void IconItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{

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

    int backgroundMargin = kIconModeColumnPadding;

    QPainterPath path = paintItemBackgroundAndGeomerty(painter, opt, index, backgroundMargin);

    QRectF iconRect = paintItemIcon(painter, opt, index);

    paintItemFileName(painter, iconRect, path, backgroundMargin, opt, index);

    painter->setOpacity(1);
}

bool IconItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(kItemFileIconModelToolTipRole).toString();

        const QList<QRect> &geometries = paintGeomertys(option, index);

        if (tooltip.isEmpty() || index == view->rootIndex() || geometries.count() < 3) {   // 当从一个需要显示tooltip的icon上移动光标到不需要显示的icon上时立即隐藏当前tooltip
            ItemDelegateHelper::hideTooltipImmediately();
        } else if (option.fontMetrics.horizontalAdvance(tooltip) > geometries[1].width() * 2) {
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

    geometries << iconRect;

    QString str = index.data(Qt::DisplayRole).toString();

    if (str.isEmpty()) {
        return geometries;
    }

    // init file name geometry
    QRect labelRect = option.rect;
    int backgroundMargin = kIconModeColumuPadding;
    labelRect.setWidth(labelRect.width() - 2 * kIconModeTextPadding - 2 * backgroundMargin - kIconModeBackRadius);
    labelRect.moveLeft(labelRect.left() + kIconModeTextPadding + backgroundMargin + kIconModeBackRadius / 2);
    labelRect.setTop(iconRect.bottom() + kIconModeTextPadding + kIconModeIconSpacing);

    QStyleOptionViewItem opt = option;

    bool isSelected = parent()->isSelected(index) && opt.showDecorationSelected;
    // if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    QTextLayout textLayout;

    textLayout.setFont(option.font);
    textLayout.setText(str);

    bool elide = (!isSelected || !singleSelected);
    QList<QRectF> lines = drawText(index, nullptr, str, QRect(labelRect.topLeft(), QSize(labelRect.width(), INT_MAX)),
                                   kIconModeRectRadius, isSelected ? opt.backgroundBrush : QBrush(Qt::NoBrush),
                                   QTextOption::WrapAtWordBoundaryOrAnywhere, elide ? opt.textElideMode : Qt::ElideNone,
                                   Qt::AlignCenter);

    labelRect = GlobalPrivate::boundingRect(lines).toRect();
    labelRect.setTop(iconRect.bottom());
    geometries << labelRect;

    // background rect
    QRect backgroundRect = option.rect;
    backgroundRect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin);
    geometries << backgroundRect;

    return geometries;
}

void IconItemDelegate::updateItemSizeHint()
{
    Q_D(IconItemDelegate);
    d->textLineHeight = parent()->parent()->fontMetrics().lineSpacing();

    int width = parent()->parent()->iconSize().width() + 30;
    int height = parent()->parent()->iconSize().height()
            + 2 * kIconModeColumnPadding   // 上下两个icon的间距
            + 3 * d->textLineHeight   // 3行文字的高度
            + 2 * kIconModeTextPadding   // 文字两边的间距
            + kIconModeIconSpacing;   // icon的间距

    int size = qMax(width, height);
    d->itemSizeHint = QSize(size, size);
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
    return iconSizeList().count() - 1;
}

int IconItemDelegate::increaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

int IconItemDelegate::decreaseIcon()
{
    Q_D(const IconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

int IconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(IconItemDelegate);

    if (level == iconSizeLevel()) {
        return level;
    }

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;
        d->itemIconSize = iconSizeByIconSizeLevel();
        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());
        return d->currentIconSizeIndex;
    }

    return -1;
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

    double iconTopOffset = (itemRect.height() - iconRect.height()) / 3.0;
    iconRect.moveLeft(itemRect.left() + (itemRect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(itemRect.top() + iconTopOffset);   // move icon down

    return iconRect;
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

void IconItemDelegate::editorFinished()
{
    FileViewHelper *viewHelper = parent();
    if (!viewHelper)
        return;

    FileView *fileview = viewHelper->parent();
    if (!fileview)
        return;

    auto windowId = WorkspaceHelper::instance()->windowId(fileview);
    if (!fileview->model())
        return;
    QUrl url = fileview->model()->getUrlByIndex(d->editingIndex);
    WorkspaceEventCaller::sendRenameEndEdit(windowId, url);
}

void IconItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_D(const IconItemDelegate);
    // Todo(yanghao):initTextLayout
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
    bool isDragMode = (static_cast<QPaintDevice *>(parent()->parent()->viewport()) != painter->device());
    bool isSelected = !isDragMode && (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor backgroundColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor baseColor = backgroundColor;
    if (option.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(option.widget);
        baseColor = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    if ((isDropTarget && !isSelected) || option.state & QStyle::StateFlag::State_Selected) {
        backgroundColor.setAlpha(backgroundColor.alpha() + 30);
    } else if (option.state & QStyle::StateFlag::State_MouseOver) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
            backgroundColor = baseColor;
        } else {
            backgroundColor = backgroundColor.lighter();
        }
    } else {
        backgroundColor = baseColor;
    }

    QRectF backgroundRect = option.rect;

    // for checkmark
    backgroundRect.adjust(backgroundMargin, backgroundMargin, -backgroundMargin, -backgroundMargin);

    // draw background
    QPainterPath path;
    backgroundRect.moveTopLeft(QPointF(0.5, 0.5) + backgroundRect.topLeft());
    path.addRoundedRect(backgroundRect, kIconModeBackRadius, kIconModeBackRadius);

    if (!isDragMode) {   // 拖拽的图标不画背景
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, backgroundColor);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    if (isDropTarget && !isSelected) {
        painter->setPen(backgroundColor);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }
    if (isSelected) {
        QRect rc = option.rect;
        rc.setSize({ 20, 20 });
        rc.moveTopRight(QPoint(option.rect.right() - 5, option.rect.top() + 5));

        DStyleOptionButton check;
        check.state = DStyle::State_Enabled | DStyle::State_On;
        check.rect = rc;

        DApplication::style()->drawPrimitive(DStyle::PE_IndicatorItemViewItemCheck, &check, painter);
    }

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
        ItemDelegateHelper::paintIcon(painter, opt.icon, iconRect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    const QUrl &url = parent()->parent()->model()->getUrlByIndex(index);
    WorkspaceEventSequence::instance()->doPaintIconItem(kItemIconRole, url, painter, &iconRect);

    paintEmblems(painter, iconRect, index);

    return iconRect;
}

void IconItemDelegate::paintItemFileName(QPainter *painter, QRectF iconRect, QPainterPath path, int backgroundMargin, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_D(const IconItemDelegate);

    if (index == d->editingIndex)
        return;   // 正在编辑的item，不重绘text

    bool isDragMode = (static_cast<QPaintDevice *>(parent()->parent()->viewport()) != painter->device());
    if (index == d->expandedIndex && !isDragMode
        && d->expandedItem && d->expandedItem->getIndex() == index
        && d->expandedItem->getOption().rect == opt.rect) {
        // fixbug65053 屏幕数据变化后，桌面展开图标的文本位置错误
        // 被展开的item，且rect未改变时，不重绘text
        d->expandedItem->setOption(opt);
        return;
    }

    // init file name geometry
    QRectF labelRect = opt.rect;
    labelRect.setTop(static_cast<int>(iconRect.bottom()) + kIconModeTextPadding + kIconModeIconSpacing);
    labelRect.setWidth(opt.rect.width() - 2 * kIconModeTextPadding - 2 * backgroundMargin - kIconModeBackRadius);
    labelRect.moveLeft(labelRect.left() + kIconModeTextPadding + backgroundMargin + kIconModeBackRadius / 2);
    labelRect.setBottom(path.boundingRect().toRect().bottom());

    const QUrl &url = parent()->parent()->model()->getUrlByIndex(index);

    if (WorkspaceEventSequence::instance()->doPaintIconItem(kItemFileDisplayNameRole, url, painter, &labelRect))
        return;

    //文管窗口拖拽时的字体保持白色
    if (isDragMode) {
        painter->setPen(opt.palette.color(QPalette::BrightText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Text));
    }

    QString displayName = displayFileName(index);

    // if has selected show all file name else show elide file name.
    bool singleSelected = parent()->parent()->selectedIndexCount() < 2;
    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    if (isSelected && singleSelected) {
        const_cast<IconItemDelegate *>(this)->hideNotEditingIndexWidget();
        /// init file name text
        const QList<QRectF> &lines = drawText(index, nullptr, displayName, labelRect.adjusted(0, 0, 0, 99999), 0, QBrush(Qt::NoBrush));
        qreal height = GlobalPrivate::boundingRect(lines).height();

        if (height > labelRect.height()) {
            d->expandedIndex = index;

            setEditorData(d->expandedItem, index);
            parent()->parent()->setIndexWidget(index, d->expandedItem);

            // 重设item状态
            d->expandedItem->setIndex(index);
            d->expandedItem->setOption(opt);
            d->expandedItem->setTextBounding(QRectF());
            d->expandedItem->setFixedWidth(0);
            double iconTopOffset = (opt.rect.height() - iconRect.height()) / 3.0;

            d->expandedItem->setContentsMargins(backgroundMargin, static_cast<int>(std::ceil(iconTopOffset)), backgroundMargin, 0);

            if (parent()->parent()->indexOfRow(index) == parent()->parent()->rowCount() - 1) {
                d->lastAndExpandedIndex = index;
            }

            parent()->updateGeometries();

            return;
        }
    } else {
        if (!singleSelected) {
            const_cast<IconItemDelegate *>(this)->hideNotEditingIndexWidget();
        }
    }

    //图标拖拽时保持活动色
    auto tempBackground = isDragMode ? (opt.palette.brush(QPalette::Normal, QPalette::Highlight)) : QBrush(Qt::NoBrush);
    drawText(index, painter, displayName, labelRect, kIconModeRectRadius, tempBackground,
             QTextOption::WrapAtWordBoundaryOrAnywhere, opt.textElideMode, Qt::AlignCenter);
}

QSize IconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const IconItemDelegate);

    int size = iconSizeList().at(d->currentIconSizeIndex);

    return QSize(size, size);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{

    Q_D(const IconItemDelegate);

    const QSize &size = d->itemSizeHint;

    if (index.isValid() && index == d->lastAndExpandedIndex) {
        d->expandedItem->setIconHeight(parent()->parent()->iconSize().height());
        return QSize(size.width(), d->expandedItem->heightForWidth(size.width()));
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
            if (FileSortFilterProxyModel *model = fileView->model()) {
                auto windowId = WorkspaceHelper::instance()->windowId(parent);
                QUrl url = model->getUrlByIndex(index);
                WorkspaceEventCaller::sendRenameStartEdit(windowId, url);
            }
        }
    }

    return editor;
}

void IconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const IconItemDelegate);

    const QSize &iconSize = parent()->parent()->iconSize();

    editor->move(option.rect.topLeft());
    editor->setMinimumHeight(option.rect.height());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (editor == d->expandedItem) {
        //重置textBounding，使其在adjustSize重新计算，否则在调整图标大小时使用旧的textBounding计算导致显示不全
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
    const int maxHeight = viewHelper ? (viewHelper->viewContentSize().height() - viewHelper->verticalOffset() - item->pos().y()) : INT_MAX;
    item->setMaxHeight(maxHeight);

    QLabel *icon = item->getIconLabel();

    if (iconSize.height() != icon->size().height()) {
        double iconTopOffset = (opt.rect.height() - iconSize.height()) / 3.0;   // update edit pos
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

    if (!item)
        return;

    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();

    QString suffix = index.data(kItemFileSuffixOfRenameRole).toString();
    qDebug() << "Display" << index.data(kItemFileDisplayNameRole).toString()
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
    } else {
        item->setProperty(kEidtorShowSuffix, suffix);
        item->setMaxCharSize(NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1));

        QString name = index.data(kItemFileBaseNameOfRenameRole).toString();
        name = FileUtils::preprocessingFileName(name);

        item->setText(name);
        item->select(name);
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
