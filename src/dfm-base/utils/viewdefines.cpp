// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewdefines.h"

DFMBASE_USE_NAMESPACE

ViewDefines::ViewDefines()
{
    initDefines();
}

int ViewDefines::iconSizeCount() const
{
    return iconSizeList.size();
}

int ViewDefines::iconSize(int index) const
{
    return iconSizeList.at(index);
}

int ViewDefines::indexOfIconSize(int size) const
{
    return iconSizeList.indexOf(size);
}

int ViewDefines::iconGridDensityCount() const
{
    return iconGridDensityList.size();
}

int ViewDefines::iconGridDensity(int index) const
{
    return iconGridDensityList.at(index);
}

int ViewDefines::indexOfIconGridDensity(int density) const
{
    return iconGridDensityList.indexOf(density);
}

int ViewDefines::listHeightCount() const
{
    return listHeightList.size();
}

int ViewDefines::listHeight(int index) const
{
    return listHeightList.at(index);
}

int ViewDefines::indexOfListHeight(int height) const
{
    return listHeightList.indexOf(height);
}

void ViewDefines::initDefines()
{
    // init icon size list
    int size = kIconSizeMin;
    // range 24-192, step size 8
    while (size <= kIconSizeMiddle) {
        iconSizeList.append(size);
        size += kIconSizeNormalStep;
    }
    // range 192-512, step size 16
    size = kIconSizeMiddle + kIconSizeLargeStep;
    while (size <= kIconSizeMax) {
        iconSizeList.append(size);
        size += kIconSizeLargeStep;
    }

    // init icon grid density
    int width = kIconGridDensityMin;
    while (width <= kIconGridDensityMax) {
        iconGridDensityList.append(width);
        width += kIconGridDensityStep;
    }

    // init list height
    listHeightList = { 24, 32, 48 };
}

