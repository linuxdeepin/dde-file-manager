/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef USER_H
#define USER_H

#include <QObject>

namespace TestObj {

class User : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "User")
    Q_PROPERTY(int id READ getId WRITE setId)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(QString password READ getPassword WRITE setPassword)
    Q_PROPERTY(QString email READ getEmail WRITE setEmail)
    Q_PROPERTY(double height READ getHeight WRITE setHeight)
    Q_PROPERTY(double weight READ getWeight WRITE setWeight)

public:
    explicit User(QObject *parent = nullptr);

    int getId() const;
    void setId(int value);

    QString getName() const;
    void setName(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);

    QString getEmail() const;
    void setEmail(const QString &value);

    double getHeight() const;
    void setHeight(double value);

    double getWeight() const;
    void setWeight(double value);

private:
    int id;   // not null, primary key autoincrement
    QString name;   // not null
    QString password;   // not null
    QString email;
    double height;
    double weight;
};
}   // namespace TestObj
#endif   // USER_H
