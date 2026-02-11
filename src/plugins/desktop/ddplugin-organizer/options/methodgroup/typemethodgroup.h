// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPEMETHODGROUP_H
#define TYPEMETHODGROUP_H

#include "methodgrouphelper.h"
#include "options/widgets/checkboxwidget.h"

namespace ddplugin_organizer {

class TypeMethodGroup : public MethodGroupHelper
{
    Q_OBJECT
public:
    explicit TypeMethodGroup(QObject *parent = nullptr);
    ~TypeMethodGroup();
    Classifier id() const override;
    void release() override;
    bool build() override;
    QList<QWidget *> subWidgets() const override;
protected slots:
    void onChanged(bool);
protected:
    QList<CheckBoxWidget *> categories;
    const QHash<ItemCategory, QString> categoryName;
};
}

#endif // TYPEMETHODGROUP_H
