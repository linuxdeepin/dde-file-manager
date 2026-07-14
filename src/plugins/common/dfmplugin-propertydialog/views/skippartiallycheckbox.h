// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SKIPPARTIALLYCHECKBOX_H
#define SKIPPARTIALLYCHECKBOX_H

#include <DCheckBox>

namespace dfmplugin_propertydialog {

class SkipPartiallyCheckBox : public DTK_WIDGET_NAMESPACE::DCheckBox
{
    Q_OBJECT
public:
    explicit SkipPartiallyCheckBox(QWidget *parent = Q_NULLPTR);

protected:
    void nextCheckState() override;
};

}

#endif
