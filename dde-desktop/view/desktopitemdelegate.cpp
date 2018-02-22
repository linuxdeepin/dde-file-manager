/*
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

#include <QAbstractItemView>
#include <dfileviewhelper.h>

#include "private/dstyleditemdelegate_p.h"

DesktopItemDelegate::DesktopItemDelegate(DFileViewHelper *parent) :
    DIconItemDelegate(parent)
{
    iconSizes << 32 << 48 << 64 << 96 << 128;
    iconSizeDescriptions << tr("Tiny")
                         << tr("Small")
                         << tr("Medium")
                         << tr("Large")
                         << tr("Super large");
}

DesktopItemDelegate::~DesktopItemDelegate()
{

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
    int width = parent()->parent()->iconSize().width() * 17 / 10;
    int height = parent()->parent()->iconSize().height()
                 + 10 + 2 * d_ptr->textLineHeight;

    d_ptr->itemSizeHint = QSize(width, height);
}
