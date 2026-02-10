// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwidget.h"

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

CheckBoxWidget::CheckBoxWidget(const QString &text, QWidget *parent)
    : EntryWidget(new DCheckBox(text), nullptr, parent)
{
    checkBox = qobject_cast<DCheckBox *>(leftWidget);
    connect(checkBox, &QCheckBox::stateChanged, this, [this](int stat) {
        emit changed(stat == Qt::Checked);
    });
}

void CheckBoxWidget::setChecked(bool checked)
{
    blockSignals(true);
    checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    blockSignals(false);
}

bool CheckBoxWidget::checked() const
{
    return checkBox->checkState() == Qt::Checked;
}
