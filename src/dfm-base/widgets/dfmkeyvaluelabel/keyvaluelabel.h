// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYVALUELABEL_H
#define KEYVALUELABEL_H

#include <dfm-base/dfm_base_global.h>

#include <DFontSizeManager>
#include <DPalette>
#include <DLabel>

#include <QFrame>
#include <QGridLayout>

DWIDGET_USE_NAMESPACE
namespace dfmbase {

class ClickableLabel : public DLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr)
        : DLabel(parent) {}

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void clicked();
};

class KeyValueLabel : public QFrame
{
    Q_OBJECT
public:
    enum KeyType {
        kLeftValue,
        kLeftElideMode,
        kLeftTipVisible,
        kLeftAlignment,
        kLeftWordWrap,
        kLeftFontWidth,
        kRightValue,
        kRightElideMode,
        kRightTipVisible,
        kRightAlignment,
        kRightWordWrap,
        kRightFontWidth
    };

    explicit KeyValueLabel(QWidget *parent);
    virtual ~KeyValueLabel() override;

private:
    void initUI();
    void initPropertyMap();
    void initFont();

private Q_SLOTS:
    void initUiForSizeMode();

public:
    void setLeftValue(QString value, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false, int fontMinWidth = 150);

    void setRightValue(QString value, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false, int fontMinWidth = 130);

    void setLeftRightValue(QString leftValue, QString rightValue, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false);

    void adjustHeight();

    void setLeftWordWrap(bool wordWrap = false);

    void setRightWordWrap(bool wordWrap = false);

    void setLeftFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    void setRightFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    QString LeftValue();

    QString RightValue();
    void setLeftVauleLabelFixedWidth(int width);

Q_SIGNALS:
    void valueAreaClicked();

protected:
    void paintEvent(QPaintEvent *evt) override;

private:
    DLabel *leftValueLabel = nullptr;
    ClickableLabel *rightValueLabel = nullptr;
    QGridLayout *glayout = nullptr;
    QMap<KeyType, QVariant> propertyMap {};
};
}
#endif   //KEYVALUELABEL_H
