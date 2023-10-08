// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENCOMBOX_H
#define SCREENCOMBOX_H

#include <com_deepin_daemon_display.h>
#include <widgets/comboxwidget.h>

#include <QTimer>

namespace dfm_wallpapersetting {

using DisplayInter = com::deepin::daemon::Display;
class ScreenComBox : public dcc::widgets::ComboxWidget
{
    Q_OBJECT
public:
    explicit ScreenComBox(QFrame *parent = nullptr);
    void reset();
    bool isSingle();
    void setCurrentScreen(const QString &name);
    inline QList<QString> getScreens() const {
        return screens;
    }
signals:
    void screenChanged(const QString &sc);
    void screenReseted();
protected slots:
    void selectChanged(int idx);
    void delay();
    void process();
private:
    QString addSuffix(const QString &name);
private:
    DisplayInter *displayInter = nullptr;
    QTimer delayTimer;
    QList<QString> screens;
};

}

#endif // SCREENCOMBOX_H
