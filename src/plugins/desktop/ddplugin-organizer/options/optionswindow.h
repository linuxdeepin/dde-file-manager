// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <DAbstractDialog>

namespace ddplugin_organizer {

class OptionsWindowPrivate;
class OptionsWindow : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
    friend class OptionsWindowPrivate;
public:
    explicit OptionsWindow(QWidget *parent = 0);
    ~OptionsWindow() override;
    bool initialize();
    void moveToCenter(const QPoint &cursorPos);
private:
    OptionsWindowPrivate *d;
};

}

#endif // OPTIONSWINDOW_H
