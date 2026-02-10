// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include "ddplugin_wallpapersetting_global.h"

#include <dtkwidget_global.h>
#include <QFrame>

class QLabel;
DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

namespace ddplugin_wallpapersetting {

class LoadingLabel : public QFrame
{
public:
    explicit LoadingLabel(QWidget *parent = nullptr);
    ~LoadingLabel();
    void start();
    void resize(const QSize &size);
    void setText(const QString &text);
private:
    void init();
    void resize(int w, int h) = delete;
private:
    QLabel *icon = nullptr;
    QLabel *contant = nullptr;
    DTK_WIDGET_NAMESPACE::DSpinner *animationSpinner = nullptr;
    double proportion = 874 / 1900.0;//根据设计计算比例
    double moveDistance = 0.0;//移动的相对位置
    QSize iconSize = QSize(30, 30);
    QSize contantSize = QSize(200, 20);
    QSize spinnerSize = QSize(24, 24);
};

}

#endif // LOADINGLABEL_H
