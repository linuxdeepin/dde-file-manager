// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef METHODCOMBOX_H
#define METHODCOMBOX_H

#include "organizer_defines.h"
#include "options/widgets/entrywidget.h"

#include <DComboBox>

#include <QLabel>

namespace ddplugin_organizer {

class MethodComBox : public EntryWidget
{
    Q_OBJECT
public:
    explicit MethodComBox(const QString &title, QWidget *parent = nullptr);
    void initCheckBox();
    void setCurrentMethod(int idx);
    int currentMethod();
signals:
    void methodChanged();
protected:
    QLabel *label = nullptr;
    Dtk::Widget::DComboBox *comboBox = nullptr;
};

}

#endif // METHODCOMBOX_H
