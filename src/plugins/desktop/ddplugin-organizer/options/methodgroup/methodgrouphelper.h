// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef METHODGROUPHELPER_H
#define METHODGROUPHELPER_H

#include "organizer_defines.h"

#include <QObject>

namespace ddplugin_organizer {

class MethodGroupHelper : public QObject
{
    Q_OBJECT
public:
    static MethodGroupHelper *create(Classifier);
    ~MethodGroupHelper() override;
    virtual Classifier id() const = 0;
    virtual void release();
    virtual bool build();
    virtual QList<QWidget*> subWidgets() const = 0;
signals:

public slots:
protected:
    explicit MethodGroupHelper(QObject *parent = nullptr);
};

}

#endif // METHODGROUPHELPER_H
