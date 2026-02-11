// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWDEFINES_H
#define VIEWDEFINES_H

#include <dfm-base/dfm_base_global.h>

#include <QList>

namespace dfmbase {
// view defines
inline constexpr int kIconSizeMax { 512 };
inline constexpr int kIconSizeMin { 24 };
inline constexpr int kIconSizeMiddle { 192 };
inline constexpr int kIconSizeNormalStep { 8 };
inline constexpr int kIconSizeLargeStep { 16 };
inline constexpr int kIconGridDensityMin { 60 };
inline constexpr int kIconGridDensityMax { 198 };
inline constexpr int kIconGridDensityStep { 6 };

class ViewDefines
{
public:
    explicit ViewDefines();

    int iconSizeCount() const;
    int iconSize(int index) const;
    int indexOfIconSize(int size) const;
    QVariantList getIconSizeList() const;
    int iconGridDensityCount() const;
    int iconGridDensity(int index) const;
    int indexOfIconGridDensity(int density) const;
    QVariantList getIconGridDensityList() const;
    int listHeightCount() const;
    int listHeight(int index) const;
    int indexOfListHeight(int height) const;
    QVariantList getListHeightList() const;
private:
    void initDefines();
    QVariantList transToVariantList(const QList<int> list) const;

    QList<int> iconSizeList {};
    QList<int> iconGridDensityList {};
    QList<int> listHeightList {};
};

}

#endif // VIEWDEFINES_H
