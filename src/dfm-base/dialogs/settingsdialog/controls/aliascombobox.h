// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALIASCOMBOBOX_H
#define ALIASCOMBOBOX_H

#include <DComboBox>

class AliasComboBox : public DTK_WIDGET_NAMESPACE::DComboBox
{
    Q_OBJECT
public:
    explicit AliasComboBox(QWidget *parent = nullptr);

    void setItemAlias(int index, const QString &alias);
    QString itemAlias(int index) const;

protected:
    void paintEvent(QPaintEvent *e) override;
};

#endif   // ALIASCOMBOBOX_H
