/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
