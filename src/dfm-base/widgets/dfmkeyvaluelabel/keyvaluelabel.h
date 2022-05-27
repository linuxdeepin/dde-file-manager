/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#ifndef KEYVALUELABEL_H
#define KEYVALUELABEL_H

#include "dfm-base/dfm_base_global.h"

#include <DFontSizeManager>
#include <DPalette>
#include <DLabel>

#include <QFrame>
#include <QGridLayout>

DWIDGET_USE_NAMESPACE
DFMBASE_BEGIN_NAMESPACE

class KeyValueLabel : public QFrame
{
    Q_OBJECT
public:
    enum KeyType {
        kLeftValue,
        kLeftElideMode,
        kLeftTip,
        kLeftAlignment,
        kLeftWordWrap,
        kLeftFontWidth,
        kRightValue,
        kRightElideMode,
        kRightTip,
        kRightAlignment,
        kRightWordWrap,
        kRightFontWidth,
        kRowHeight
    };

    explicit KeyValueLabel(QWidget *parent);
    virtual ~KeyValueLabel() override;

private:
    void initUI();

    void initPropertyMap();

public:
    void setLeftValue(QString value, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false, int fontMinWidth = 150);

    void setRightValue(QString value, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false, int fontMinWidth = 130);

    void setLeftRightValue(QString leftValue, QString rightValue, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false);

    void setRowMinimumHeight(int height);

    void setLeftWordWrap(bool wordWrap = false);

    void setRightWordWrap(bool wordWrap = false);

    void setLeftFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    void setRightFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    QString LeftValue();

    QString RightValue();

protected:
    void paintEvent(QPaintEvent *evt) override;

private:
    DLabel *leftValueLabel = nullptr;
    DLabel *rightValueLabel = nullptr;
    QGridLayout *glayout = nullptr;
    QMap<KeyType, QVariant> propertyMap {};
};
DFMBASE_END_NAMESPACE
#endif   //KEYVALUELABEL_H
