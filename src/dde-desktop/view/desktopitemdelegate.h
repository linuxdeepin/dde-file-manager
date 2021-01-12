/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#pragma once

#include <diconitemdelegate.h>

class DesktopItemDelegate : public DIconItemDelegate
{
    Q_OBJECT
public:
    explicit DesktopItemDelegate(DFileViewHelper *parent);
    virtual ~DesktopItemDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    QString iconSizeLevelDescription(int i) const;
    int iconSizeLevel() const override;
    int minimumIconSizeLevel() const override;
    int maximumIconSizeLevel() const override;

    int increaseIcon() override;
    int decreaseIcon() override;
    int setIconSizeByIconSizeLevel(int level) override;
    QSize iconSizeByIconSizeLevel() const;

    void updateItemSizeHint() override;
    void hideAllIIndexWidget() override;
private:
    QStringList iconSizeDescriptions;
    QList<int> iconSizes;
    // default icon size is 48px.
    int currentIconSizeIndex = -1;
};

