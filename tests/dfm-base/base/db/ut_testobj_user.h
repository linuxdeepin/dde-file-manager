// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
