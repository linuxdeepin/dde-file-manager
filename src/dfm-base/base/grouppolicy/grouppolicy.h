/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef GROUPPOLICY_H
#define GROUPPOLICY_H

#include "dfm_base_global.h"

#include <dtkcore_global.h>

#include <QObject>
#include <QVariant>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

namespace dfmbase {

class GroupPolicy : public QObject
{
    Q_OBJECT
public:
    static GroupPolicy *instance();

    QStringList keys();
    bool contains(const QString &key);
    QVariant value(const QString &key, const QVariant &fallback = QVariant());
    void setValue(const QString &key, const QVariant &value);

signals:
    void valueChanged(const QString &key);

protected:
    explicit GroupPolicy(QObject *parent = nullptr);
    bool isValidConfig();

private:
    Dtk::Core::DConfig *config;
};

}

#endif   // GROUPPOLICY_H
