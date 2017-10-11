/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef HOTZONE_H
#define HOTZONE_H

#include "hoverwidget.h"
#include "pushbuttonlist.h"

#include <QWidget>
#include <QMatrix>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QPoint>
#include <QLabel>
#include <QPushButton>
#include <QList>

class HotZone : public QWidget
{
    Q_OBJECT
public:
    HotZone(QWidget *parent, bool isRight, bool isBottom);
    ~HotZone();

    void addButton(QString buttonName, QString actionStr, bool lastAddedButton);
    void addButtons(QStringList buttonNames,QStringList actionStrs);
private slots:
    void onMouseHasEntered();
    void onMouseHasLeaved();
    void onMouseHasLeavedTheColumnItem();

private:
    void initSeveralVar(bool isRight, bool m_isBottom);
    void initItemAtCorrectCorner(QWidget* mainWindow);
    void initImageAndTextAtCorrectCorner();
    void initButtonColumnAtCorrectCorner();
    void initAnimationCorrectly();
    void initConnects();

    void rePositionButtonColumn(int BottomOffset, int RightOffset);
    void reSetAnimation();

    QPixmap getPixmap(const QString &file);

    void updateEnterPic();
    void updateLeavePic();

    QWidget *m_mainItem;
    QLabel *m_selectedActionTextLabel;
    QLabel *m_imageLabel;
    QPixmap m_pixmap;
    HoverWidget *m_hoverItem;
    HoverWidget *m_ButtonColumnItem;
    QMatrix *m_matrix;
    PushButtonList *m_pushButtonList;
    QVBoxLayout *m_vLayout;
    QPropertyAnimation *m_animation;
    QPoint m_startPoint;
    QPoint m_endPoint;

    QList< QPushButton* > m_actionButtonList;

    enum Corner{
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    } corner;

    int m_ButtonsNum;
    int m_ButtonColumnBottomDistance;
    int m_ButtonColumnRightDistance;

    bool m_isRight;
    bool m_isBottom;

    bool m_hasAnimationRun;
    bool m_mouseIsEnterFromButtonColumnAfterButtonClick;

    // this variable indicate the hight and width of m_mainItem;
    const int WRAPPER_SIZE = 300;
    // imageSize indicates the height and width of image ,also the size of m_hoverItem
    const int IMAGE_SIZE = 110;
    // MAIN_ITEM_TOP_MARGIN is aimed to steer clear of the fade-zone of mouseEvent of mainWindow Item.
    const int MAIN_ITEM_TOP_MARGIN = 30;
    // these three constants are used to justify m_selectedActionTextLabel's position
    const int TEXT_LEFT_OR_RIGHT_MARGIN = 36 ;
    const int TEXT_TOP_MARGIN = 10;
    const int TEXT_BOTTOM_DISTANCE = 25 ;

    const int BUTTON_HEIGHT = 24;
    // set down m_selectedActionTextLabel's fixedWidth
    const int SELECTED_LABEL_FIXED_WIDTH = 150;
    // set down m_actionColumnItem's leftOrRight and topOrBottom Margin
    const int BUTTON_COLUMN_LEFT_DISTANCE = 30;
//    const int BUTTON_COLUMN_RIGHT_DISTANCE =  91;
    const int BUTTON_COLUMN_TOP_DISTANCE = 36;
    const int ANIMATION_MOVE_DISTANCE = 20;
};

#endif // HOTZONE_H
