// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/delegatecommon.h"
#include "private/listitemdelegate_p.h"
#include "listitemdelegate.h"
#include "fileview.h"
#include "listitemeditor.h"
#include "abstractitempaintproxy.h"
#include "models/fileviewmodel.h"
#include <dfm-base/dfm_base_global.h>
#include "utils/itemdelegatehelper.h"
#include "utils/fileviewhelper.h"
#include "events/workspaceeventcaller.h"
#include "events/workspaceeventsequence.h"
#include "utils/workspacehelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/elidetextlayout.h>

#include <DListView>
#include <DArrowRectangle>
#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainterPath>

#include <linux/limits.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

ListItemDelegate::ListItemDelegate(FileViewHelper *parent)
    : BaseItemDelegate(*new ListItemDelegatePrivate(this), parent)
{
    parent->parent()->setIconSize(QSize(kListViewIconSize,
                                        kListViewIconSize));
}

ListItemDelegate::~ListItemDelegate()
{
}

void ListItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;

    auto info = parent()->fileInfo(index);
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont oldFont = option.font;

    if (oldFont != option.font) {
        QWidget *editingWidget = editingIndexWidget();

        if (editingWidget) {
            editingWidget->setFont(option.font);
        }

        const_cast<ListItemDelegate *>(this)->updateItemSizeHint();
    }

    oldFont = option.font;

    if (parent()->isTransparent(index)) {
        painter->setOpacity(0.3);
    }

    paintItemBackground(painter, opt, index);

    QRectF iconRect = paintItemIcon(painter, opt, index);

    paintItemColumn(painter, opt, index, iconRect);

    painter->setOpacity(1);
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_D(const ListItemDelegate);

    // Todo(yanghao): isColumnCompact (fontMetrics.height() * 2 + 10)
    return QSize(d->itemSizeHint.width(), qMax(option.fontMetrics.height(), d->itemSizeHint.height()));
}

QWidget *ListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    D_DC(ListItemDelegate);
    Q_UNUSED(option);

    d->editingIndex = index;
    d->editor = new ListItemEditor(parent);

    const FileInfoPointer &fileInfo = this->parent()->fileInfo(index);

    if (fileInfo && fileInfo->urlOf(UrlInfoType::kUrl).scheme() == "search") {
        d->editor->setFixedHeight(GlobalPrivate::kListEditorHeight * 2 - 10);
    } else {
        d->editor->setFixedHeight(GlobalPrivate::kListEditorHeight);
    }

    connect(static_cast<ListItemEditor *>(d->editor), &ListItemEditor::inputFocusOut, this, &ListItemDelegate::editorFinished);

    connect(d->editor, &QLineEdit::destroyed, this, [=] {
        d->editingIndex = QModelIndex();
        d->editor = nullptr;
    });

    auto windowId = WorkspaceHelper::instance()->windowId(parent);
    QUrl url = this->parent()->parent()->model()->data(index, kItemUrlRole).toUrl();
    WorkspaceEventCaller::sendRenameStartEdit(windowId, url);

    return d->editor;
}

void ListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect &optRect = option.rect + QMargins(-kListModeLeftMargin - kListModeLeftPadding, 0, -kListModeRightMargin - kListModeRightMargin, 0);
    QRect iconRect = getRectOfItem(RectOfItemType::kItemIconRect, index);

    const QList<ItemRoles> &columnRoleList = parent()->parent()->model()->getColumnRoles();
    if (columnRoleList.isEmpty())
        return;

    int columnX = 0;
    QRect rect = optRect;
    for (int i = 0; i < columnRoleList.length(); ++i) {
        int rol = columnRoleList.at(i);
        if (rol == kItemFileDisplayNameRole) {
            int iconOffset = i == 0 ? iconRect.right() + 1 : 0;

            rect.setLeft(columnX + iconOffset);
            columnX += parent()->parent()->getColumnWidth(i) - 1 - parent()->fileViewViewportMargins().left();

            rect.setRight(qMin(columnX, optRect.right()));
            rect.setTop(optRect.y() + (optRect.height() - editor->height()) / 2);
            break;
        } else {
            columnX += parent()->parent()->getColumnWidth(i);
        }
    }

    editor->setGeometry(rect);
}

void ListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // 这里设置了光标选中位置后最终还是会被全选，移到eventFilter中处理
    return QAbstractItemDelegate::setEditorData(editor, index);
}

bool ListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        //在此处处理的逻辑是因为默认QAbstractItemView的QLineEdit重命名会被SelectAll
        if (!setEditorData(qobject_cast<ListItemEditor *>(object)))
            return false;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);
        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();
            QLineEdit *edit = qobject_cast<QLineEdit *>(object);
            if (edit) {
                edit->close();
                edit->parentWidget()->setFocus();
            }
            return true;
        }
        if (e->key() == Qt::Key_Tab) {   // no item switch when tab triggered while rename an item. same behavious as icondelegate
            this->commitDataAndCloseActiveEditor();
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

bool ListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QList<ItemRoles> columnRoleList = parent()->parent()->getColumnRoles();
        if (columnRoleList.length() < 2)
            return true;

        const QList<QRect> &geometries = paintGeomertys(option, index);

        QString tooltip {};
        // 从1开始是为了排除掉icon区域
        for (int i = d->paintProxy->iconRectIndex() + 1; i < geometries.length() && i <= columnRoleList.length(); ++i) {
            const QRect &rect = geometries.at(i);

            if (rect.left() <= event->x() && rect.right() >= event->x()) {
                const QString &tipStr = index.data(columnRoleList[i - d->paintProxy->iconRectIndex() - 1]).toString();

                if (option.fontMetrics.horizontalAdvance(tipStr) > rect.width()) {
                    tooltip = tipStr;
                    break;
                }
            }
        }

        if (tooltip.isEmpty() || index == view->rootIndex()) {
            ItemDelegateHelper::hideTooltipImmediately();
        } else {
            int tooltipSize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipSize / nlong + 1;
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

void ListItemDelegate::editorFinished()
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
    QUrl url = fileview->model()->data(d->editingIndex, kItemUrlRole).toUrl();
    WorkspaceEventCaller::sendRenameEndEdit(windowId, url);
}

QList<QRect> ListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    QList<QRect> geomertys;
    const QList<ItemRoles> &columnRoleList = parent()->parent()->model()->getColumnRoles();
    int columnX = 0;

    QStyleOptionViewItem opt = option;

    geomertys.append(d->paintProxy->allPaintRect(opt, index));

    columnX = geomertys.first().right() + GlobalPrivate::kIconSpacing;

    QRect rect = opt.rect;
    rect.setLeft(columnX);

    int role = columnRoleList.at(0);

    if (sizeHintMode) {
        rect.setWidth(dataWidth(option, index, role));
        columnX = rect.right();
    } else {
        columnX = parent()->parent()->getColumnWidth(0) - 1 - parent()->fileViewViewportMargins().left();

        rect.setRight(qMin(columnX, opt.rect.right()));
        /// draw file name label
        rect.setWidth(qMin(rect.width(), dataWidth(option, index, role)));
    }

    geomertys << rect;

    for (int i = 1; i < columnRoleList.count(); ++i) {
        QRect rec = opt.rect;

        rec.setLeft(columnX + kColumnPadding);

        if (rec.left() >= opt.rect.right()) {
            return geomertys;
        }

        int rol = columnRoleList.at(i);

        if (sizeHintMode) {
            rec.setWidth(dataWidth(option, index, rol));
            columnX += rec.width();
        } else {
            columnX += parent()->parent()->getColumnWidth(i) - 1;

            rec.setRight(qMin(columnX, opt.rect.right()));
            rec.setWidth(qMin(rec.width(), dataWidth(option, index, rol)));
        }

        geomertys << rec;
    }

    return geomertys;
}

void ListItemDelegate::updateItemSizeHint()
{
    Q_D(ListItemDelegate);

    d->textLineHeight = parent()->parent()->fontMetrics().height();
    d->itemSizeHint = QSize(-1, qMax(int(parent()->parent()->iconSize().height() * 1.33), d->textLineHeight));
}

QRectF ListItemDelegate::itemIconRect(const QRectF &itemRect) const
{
    QRectF iconRect = itemRect;

    iconRect += QMargins(-kListModeLeftMargin, 0, -kListModeRightMargin, 0);
    iconRect.setLeft(iconRect.left() + kListModeLeftPadding);
    iconRect.setRight(iconRect.right() - kListModeRightPadding);

    iconRect.setSize(parent()->parent()->iconSize());

    return iconRect;
}

QRect ListItemDelegate::getRectOfItem(RectOfItemType type, const QModelIndex &index) const
{
    return d->paintProxy->rectByType(type, index).toRect();
}

/*!
 * \brief paintItemBackground 绘制listviewitemd的交替绘制和选中时的高亮绘制
 *
 * \return void
 **/
void ListItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    FileView *view = parent()->parent();
    if (!view)
        return;

    int totalWidth = view->getHeaderViewWidth() - (kListModeLeftMargin + kListModeRightMargin);

    QRectF rect = option.rect;
    rect.setLeft(rect.left() + kListModeLeftMargin);
    rect.setWidth(totalWidth);

    // draw background
    if (option.widget) {
        DPalette pl(DPaletteHelper::instance()->palette(option.widget));
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QColor adjustColor = baseColor;

        bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
        bool isDropTarget = parent()->isDropTarget(index);

        if (isSelected || isDropTarget) {
            // set highlight color
            QPalette::ColorGroup colorGroup = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
                    ? QPalette::Normal
                    : QPalette::Disabled;

            if (colorGroup == QPalette::Normal && !(option.state & QStyle::State_Active))
                colorGroup = QPalette::Inactive;

            adjustColor = option.palette.color(colorGroup, QPalette::Highlight);
        } else if (option.state & QStyle::StateFlag::State_MouseOver) {
            // hover color
            adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        } else {
            // alternately background color
            painter->setOpacity(0);
            if (index.row() % 2 == 0) {
                adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +2);
                painter->setOpacity(1);
            }
        }

        // set paint path
        QPainterPath path;
        path.addRoundedRect(rect, kListModeRectRadius, kListModeRectRadius);

        // set render antialiasing
        painter->setRenderHints(QPainter::Antialiasing
                                | QPainter::TextAntialiasing
                                | QPainter::SmoothPixmapTransform);

        painter->fillPath(path, adjustColor);
    }

    painter->restore();
}

/*!
 * \brief paintItemIcon 绘制listviewitemd的icon
 *
 * \return QRect 返回绘制icon的区域方便后面绘制
 **/
QRectF ListItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!parent() || !parent()->parent() || !d->paintProxy)
        return QRect();

    // draw icon
    QRectF iconRect = option.rect;
    d->paintProxy->drawIcon(painter, &iconRect, option, index);
    paintEmblems(painter, iconRect, index);

    return iconRect;
}
/*!
 * \brief paintItemIcon 绘制listviewitemd的剩余每一列的数据
 *
 * \return void
 **/
void ListItemDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &iconRect) const
{
    D_DC(ListItemDelegate);
    // 绘制需要绘制的项，计算每一项绘制的宽度
    const QList<ItemRoles> &columnRoleList = parent()->parent()->getColumnRoles();
    if (columnRoleList.isEmpty())
        return;

    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);

    double columnX = iconRect.right();

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));

    // 绘制那些需要显示的项
    for (int i = 0; i < columnRoleList.count(); ++i) {
        int columnWidth = parent()->parent()->getColumnWidth(i);
        if (columnWidth <= 0)
            continue;

        QRectF columnRect = opt.rect;
        columnRect.setLeft(columnX + kListModeColumnPadding);

        if (columnRect.left() >= columnRect.right())
            break;

        if (i == 0) {
            columnX = option.rect.x() + columnWidth - 1 - parent()->parent()->viewportMargins().left();
        } else {
            columnX += columnWidth;
        }

        columnRect.setRight(qMin(columnX, static_cast<qreal>(opt.rect.right() - kListModeRightMargin)));

        int rol = columnRoleList.at(i);
        const QVariant &data = index.data(rol);

        const FileInfoPointer &info = parent()->fileInfo(index);
        if (WorkspaceEventSequence::instance()->doPaintListItem(rol, info, painter, &columnRect))
            continue;

        QPalette::ColorGroup cGroup = QPalette::Inactive;
        Qt::TextElideMode elideMode = Qt::ElideRight;

        QRectF textRect = columnRect;
        textRect.setHeight(d->textLineHeight);
        textRect.moveTop(((columnRect.height() - textRect.height()) / 2) + columnRect.top());

        const QUrl &url = parent()->parent()->model()->data(index, kItemUrlRole).toUrl();
        if (rol == kItemNameRole || rol == kItemFileDisplayNameRole) {
            paintFileName(painter, opt, index, rol, textRect, d->textLineHeight, url);
        } else {
            if (!isSelected)
                painter->setPen(opt.palette.color(cGroup, QPalette::Text));

            if (data.canConvert<QString>()) {
                QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(index.data(rol).toString().remove('\n'),
                                                                                            QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                            d->textLineHeight, index.data(Qt::TextAlignmentRole).toInt(),
                                                                                            painter));
                layout->layout(textRect, elideMode, painter);
            }
        }
    }
}

void ListItemDelegate::paintFileName(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const int &role, const QRectF &rect, const int &textLineHeight,
                                     const QUrl &url) const
{
    bool drawBackground = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    const QVariant &data = index.data(role);
    painter->setPen(option.palette.color(drawBackground ? QPalette::BrightText : QPalette::Text));

    QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout("", QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                textLineHeight, index.data(Qt::TextAlignmentRole).toInt(),
                                                                                painter));

    if (data.canConvert<QString>()) {
        QString fileName {};

        if (Q_LIKELY(!FileUtils::isDesktopFileSuffix(url))) {
            do {
                if (role != kItemNameRole && role != kItemFileDisplayNameRole)
                    break;

                if (role == kItemFileDisplayNameRole) {
                    const auto itemFileName = index.data(kItemNameRole);
                    const auto itemFileDisplayName = index.data(kItemFileDisplayNameRole);

                    if (itemFileName != itemFileDisplayName)
                        break;
                }

                const QString &suffix = "." + index.data(kItemFileSuffixRole).toString();
                if (suffix == ".")
                    break;

                QStringList textList {};
                layout->setText(index.data(kItemFileBaseNameRole).toString().remove('\n'));
                QRectF baseNameRect = rect;
                baseNameRect.adjust(0, 0, -option.fontMetrics.horizontalAdvance(suffix), 0);
                layout->layout(baseNameRect, Qt::ElideRight, nullptr, Qt::NoBrush, &textList);

                fileName = textList.join('\n');

                bool showSuffix { Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool() };
                if (showSuffix)
                    fileName.append(suffix);
            } while (false);
        }

        if (fileName.isEmpty()) {
            QStringList textList {};
            layout->setText(index.data(role).toString().remove('\n'));
            layout->layout(rect, Qt::ElideRight, nullptr, Qt::NoBrush, &textList);

            fileName = fileName = textList.join('\n');
        }
        QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(fileName,
                                                                                    QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                    textLineHeight, index.data(Qt::TextAlignmentRole).toInt(),
                                                                                    painter));
        layout->layout(rect, Qt::ElideRight, painter);
    } else {
        // Todo(yanghao&liuyangming)???
        //         drawNotStringData(option, textLineHeight, rect, data, drawBackground, painter, 0);
    }
}

bool ListItemDelegate::setEditorData(ListItemEditor *editor)
{
    if (!editor)
        return false;

    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();

    const QString &suffix = d->editingIndex.data(kItemFileSuffixOfRenameRole).toString();
    const QUrl &fileUrl = d->editingIndex.data(kItemUrlRole).toUrl();

    // if file is in dlnfs' path, use char count rather than byte count to limit the filename length
    if (FileUtils::supportLongName(fileUrl))
        editor->setCharCountLimit();

    if (showSuffix) {
        QString name = d->editingIndex.data(kItemFileNameOfRenameRole).toString();
        name = FileUtils::preprocessingFileName(name);
        editor->setMaxCharSize(NAME_MAX);
        editor->setText(name);
        editor->select(name.left(name.size() - suffix.size() - (suffix.isEmpty() ? 0 : 1)));
    } else {
        editor->setProperty(kEidtorShowSuffix, suffix);
        editor->setMaxCharSize(NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1));
        QString name = d->editingIndex.data(kItemFileBaseNameOfRenameRole).toString();
        name = FileUtils::preprocessingFileName(name);
        editor->setText(name);
        editor->select(name);
    }
    return true;
}

int ListItemDelegate::dataWidth(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const
{
    const QVariant &data = index.data(role);

    if (data.canConvert<QString>()) {
        return option.fontMetrics.horizontalAdvance(data.toString());
    }

    if (data.canConvert<QPair<QString, QString>>()) {
        const QPair<QString, QString> &string_string = qvariant_cast<QPair<QString, QString>>(data);

        return qMax(option.fontMetrics.horizontalAdvance(string_string.first), option.fontMetrics.horizontalAdvance(string_string.second));
    }

    if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        const QPair<QString, QPair<QString, QString>> &string_p_string = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        return option.fontMetrics.horizontalAdvance(string_p_string.first);
    }

    return -1;
}
