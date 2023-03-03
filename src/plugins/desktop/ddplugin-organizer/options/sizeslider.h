// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIZESLIDER_H
#define SIZESLIDER_H

#include "widgets/contentbackgroundwidget.h"

#include <DSlider>

class QLabel;

namespace ddplugin_organizer {

class SizeSlider : public ContentBackgroundWidget
{
    Q_OBJECT

public:
    enum Mode {View, Icon };
public:
    explicit SizeSlider(QWidget *parent = nullptr);
    ~SizeSlider();
    void switchMode(Mode mode);
    inline Mode mode() const { return curMode;}
protected slots:
    void setIconLevel(int);
    void syncIconLevel(int);
    void iconClicked(DTK_WIDGET_NAMESPACE::DSlider::SliderIcons icon, bool checked);
protected:
    void setValue(int);
    void resetToView();
    void resetToIcon();
    int iconLevel();
    static QStringList ticks(int count);
private:
    DTK_WIDGET_NAMESPACE::DSlider *slider = nullptr;
    QLabel *label = nullptr;
    Mode curMode = View;
};

}

#endif // SIZESLIDER_H
