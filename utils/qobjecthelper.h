/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef QOBJECTHELPER_H
#define QOBJECTHELPER_H

#include <QtCore/QLatin1String>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE


class QObjectHelper
{
public:
    QObjectHelper();
    ~QObjectHelper();


    static QVariantMap qobject2qvariant(const QObject *object,
                                        const QStringList &ignoredProperties = QStringList(QString(QLatin1String("objectName"))));


    static QString qobject2json(const QObject *object,
                                const QStringList &ignoredProperties = QStringList(QString(QLatin1String("objectName"))));


    static void qvariant2qobject(const QVariantMap &variant, QObject *object);

    static void json2qobject(const QString &json, QObject *object);

private:
    QObjectHelper(QObjectHelper &) = delete;
    QObjectHelper &operator=(QObjectHelper &) = delete;
    Q_DISABLE_COPY(QObjectHelper)
    class QObjectHelperPrivate;
    QObjectHelperPrivate *const d;
};


#endif // QOBJECTHELPER_H
