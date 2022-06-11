/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "desktopitemdelegate.h"
#include "app/define.h"
#include "canvasgridview.h"
#include "canvasviewhelper.h"
#include "views/fileitem.h"

#include "private/dstyleditemdelegate_p.h"

#include <QAbstractItemView>
#include <QTextEdit>
#include <DStyle>

DesktopItemDelegate::DesktopItemDelegate(DFileViewHelper *parent) :
    DIconItemDelegate(parent)
{
    iconSizes << 32 << 48 << 64 << 96 << 128;
    // word number of per line
    charOfLine << 9 << 9 << 9 << 12 << 12;
    iconSizeDescriptions << tr("Tiny")
                         << tr("Small")
                         << tr("Medium")
                         << tr("Large")
                         << tr("Super large");
}

DesktopItemDelegate::~DesktopItemDelegate()
{

}

QWidget *DesktopItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    auto widget = DIconItemDelegate::createEditor(parent, opt, index);
    FileIconItem *item = static_cast<FileIconItem *>(widget);
    connect(item, &FileIconItem::inputFocusOut, this, &DesktopItemDelegate::commitDataAndCloseActiveEditor);
    item->setContentsMargins(0, 0, 0, 0); // no margins

    auto edit = item->getTextEdit();
    // 设置文本与编辑框的边距
    edit->document()->setDocumentMargin(TEXT_PADDING);

    //在DTextEdit中FrameRadius会被设置为viewportMargins,导致文本可用的宽度减少。
    //在此修改为0，并使用eventfilter重写绘制
    DTK_WIDGET_NAMESPACE::DStyle::setFrameRadius(edit, 0);

    // 在edit自身的EventFilter中处理绘制
    edit->installEventFilter(edit);

    return widget;
}

void DesktopItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    auto view = dynamic_cast<CanvasGridView *>(parent()->parent());
    Q_ASSERT(view);

    // get the cell rect
    opt.rect = view->visualRect(index).marginsRemoved(view->cellMargins());

    DIconItemDelegate::updateEditorGeometry(editor, opt, index);
}

QString DesktopItemDelegate::iconSizeLevelDescription(int i) const
{
    return iconSizeDescriptions.at(i);
}

int DesktopItemDelegate::iconSizeLevel() const
{
    return currentIconSizeIndex;
}

int DesktopItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int DesktopItemDelegate::maximumIconSizeLevel() const
{
    return iconSizes.count() - 1;
}

int DesktopItemDelegate::increaseIcon()
{
    return setIconSizeByIconSizeLevel(currentIconSizeIndex + 1);

}

int DesktopItemDelegate::decreaseIcon()
{
    return setIconSizeByIconSizeLevel(currentIconSizeIndex - 1);
}

int DesktopItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    if (level == currentIconSizeIndex) {
        return level;
    }

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        currentIconSizeIndex = level;

        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());

        return currentIconSizeIndex;
    }

    return -1;
}

QSize DesktopItemDelegate::iconSizeByIconSizeLevel() const
{
    int size = iconSizes.at(currentIconSizeIndex);
    return QSize(size, size);
}

void DesktopItemDelegate::updateItemSizeHint()
{
    DIconItemDelegate::updateItemSizeHint();
    auto iconSize = parent()->parent()->iconSize();
    int width;
    if (sizeMode != IconSizeMode::WordNum) {
       width = iconSize.width() * 17 / 10;
    } else {
        // update word width
        textFontWidth = parent()->parent()->fontMetrics().width("中");

        // defalut word num
        const int minWidth = iconSize.width() + ICON_TOP_SPACE_DESKTOP * 2;
        int num = 9;
        int index = iconSizes.indexOf(iconSize.width());
        if (index >= 0 && index < charOfLine.size())
            num = charOfLine.at(index);

        width = TEXT_PADDING + num * textFontWidth + TEXT_PADDING;
        if (Q_UNLIKELY(width < minWidth))
            width = minWidth;
    }

    int height = iconSize.height() + ICON_BOTTOM_SPACING_DESKTOP + TEXT_PADDING + 2 * d_ptr->textLineHeight + TEXT_PADDING;
    d_ptr->itemSizeHint = QSize(width, height);
}

void DesktopItemDelegate::hideAllIIndexWidget()
{
    //父类的hideAllIIndexWidget不会立即隐藏editor，这里做特殊处理
    auto editor = editingIndexWidget();
    DIconItemDelegate::hideAllIIndexWidget();

    if (editor)
        editor->hide();
}

void DesktopItemDelegate::setIconSizeMode(IconSizeMode mode)
{
    sizeMode = mode;
}

DesktopItemDelegate::IconSizeMode DesktopItemDelegate::iconSizeMode() const
{
    return sizeMode;
}
