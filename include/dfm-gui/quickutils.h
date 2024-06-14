// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKUTILS_H
#define QUICKUTILS_H

#include <dfm-gui/dfm_gui_global.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE

class QuickUtils : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief 用于标识窗体类型
     *  QML中 ENUM 必须以大写开头
     */
    enum WidgetType {
        UnknownType,
        Sidebar,
        Titlebar,
        DetailSpace,
        WorkSpace,
        TypeCount,
    };
    Q_ENUM(WidgetType)

    explicit QuickUtils(QObject *parent = nullptr);
};

DFMGUI_END_NAMESPACE

#endif   // QUICKUTILS_H
