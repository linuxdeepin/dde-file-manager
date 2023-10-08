// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <widgets/settingsitem.h>

namespace dfm_wallpapersetting {

class CustomButton : public dcc::widgets::SettingsItem
{
    Q_OBJECT
public:
    explicit CustomButton(QWidget *parent = nullptr);
signals:
    void clicked();
};

}
#endif // CUSTOMBUTTON_H
