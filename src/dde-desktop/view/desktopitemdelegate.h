// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <diconitemdelegate.h>

class DesktopItemDelegate : public DIconItemDelegate
{
    Q_OBJECT
public:
    enum class IconSizeMode{IconLevel = 0, WordNum};
public:
    explicit DesktopItemDelegate(DFileViewHelper *parent);
    virtual ~DesktopItemDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
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

    void setIconSizeMode(IconSizeMode mode);
    IconSizeMode iconSizeMode() const;
private:
    QStringList iconSizeDescriptions;
    QList<int> charOfLine;
    QList<int> iconSizes;
    int textFontWidth = 0;

    // default icon size is 48px.
    int currentIconSizeIndex = -1;
    IconSizeMode sizeMode = IconSizeMode::IconLevel;
};

