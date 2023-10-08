// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODEBUTTONBOX_H
#define MODEBUTTONBOX_H

#include <DButtonBox>

namespace dfm_wallpapersetting {

class ModeButtonBox : public DTK_WIDGET_NAMESPACE::DButtonBox
{
    Q_OBJECT
public:
    enum Mode{Picture = 0, SolidColor};
    explicit ModeButtonBox(QWidget *parent);
    void initialize();
    void switchMode(Mode m);
    inline Mode currentMode() const {
        return current;
    }
signals:
    void switchTo(int m);
private slots:
    void onButtonClicked(QAbstractButton *value);
private:
    Mode current = Picture;
};
}

#endif // MODEBUTTONBOX_H
