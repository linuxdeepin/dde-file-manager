/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef DESKTOP_ACCESSIBLE_LIST_H
#define DESKTOP_ACCESSIBLE_LIST_H

#include "frameaccessibledefine.h"
#include "accessible/accessibledefine.h"
#include "../view/canvasgridview.h"
#include "../view/watermaskframe.h"

#include "../dde-wallpaper-chooser/frame.h"

#include <QDialog>
#include <QPushButton>
#include <QListWidget>
#include <DTitlebar>
#include <QFrame>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

// 添加accessible
SET_FORM_ACCESSIBLE(CanvasGridView,OBJ_NAME_CANVAS_GRID_VIEW)
SET_FORM_ACCESSIBLE(WaterMaskFrame,OBJ_NAME_WATER_MASK_FRAME)
SET_FORM_ACCESSIBLE(QFrame,m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget,m_w->objectName())
SET_LABEL_ACCESSIBLE(QLabel,m_w->objectName())
SET_FORM_ACCESSIBLE(QDialog,m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton,m_w->objectName())
SET_SLIDER_ACCESSIBLE(DMainWindow,m_w->objectName())
SET_SLIDER_ACCESSIBLE(QListWidget,m_w->objectName())
SET_FORM_ACCESSIBLE(DTitlebar,m_w->objectName())
SET_FORM_ACCESSIBLE(Frame,m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(classname, CanvasGridView);
    USE_ACCESSIBLE(classname, WaterMaskFrame);
    USE_ACCESSIBLE(classname, QFrame);
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, QLabel);
    USE_ACCESSIBLE(classname, QDialog);
    USE_ACCESSIBLE(classname, QPushButton);
    USE_ACCESSIBLE(classname, DMainWindow);
    USE_ACCESSIBLE(classname, QListWidget);
    USE_ACCESSIBLE(classname, DTitlebar);
    USE_ACCESSIBLE(classname, Frame);

    return interface;
}

#endif // DESKTOP_ACCESSIBLE_LIST_H
