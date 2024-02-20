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
#include <QTextEdit>

DWIDGET_USE_NAMESPACE
namespace dfmbase {

class RightValueWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit RightValueWidget(QWidget *parent = nullptr);
    void setCompleteText(const QString &text);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusOutEvent(QFocusEvent *e) override;

private Q_SLOTS:
    void customContextMenuEvent(const QPoint &pos);

Q_SIGNALS:
    void clicked();

private:
    QString completeText;
    bool isContextMenuShow { false };
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

    void setRightValue(QString value, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::AlignHCenter, bool toolTipVisibility = false, int fontMinWidth = 130);

    void setLeftRightValue(QString leftValue, QString rightValue, Qt::TextElideMode elideMode = Qt::ElideNone, Qt::Alignment aligment = Qt::Alignment(), bool toolTipVisibility = false);

    void adjustHeight();

    void setLeftWordWrap(bool wordWrap = false);

    void setLeftFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    void setRightFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight = QFont::Normal, DPalette::ColorType foregroundRole = DPalette::NoType);

    QString LeftValue();

    QString RightValue();

    DLabel *leftWidget();

    RightValueWidget *rightWidget();

    void setLeftVauleLabelFixedWidth(int width);

Q_SIGNALS:
    void valueAreaClicked();

private:
    DLabel *leftValueLabel = nullptr;
    RightValueWidget *rightValueEdit = nullptr;
    QGridLayout *glayout = nullptr;
    QMap<KeyType, QVariant> propertyMap {};
};
}
#endif   //KEYVALUELABEL_H
