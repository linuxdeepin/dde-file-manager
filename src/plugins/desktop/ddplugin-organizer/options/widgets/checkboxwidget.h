// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWIDGET_H
#define CHECKBOXWIDGET_H

#include "entrywidget.h"

#include <DCheckBox>

namespace ddplugin_organizer {

class CheckBoxWidget : public EntryWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidget(const QString &text, QWidget *parent = nullptr);
    void setChecked(bool checked = true);
    bool checked() const;
signals:
    void changed(bool checked);

protected:
    DTK_WIDGET_NAMESPACE::DCheckBox *checkBox = nullptr;
};

}

#endif   // CHECKBOXWIDGET_H
