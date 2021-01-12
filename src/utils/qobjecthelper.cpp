/* This file is part of qjson
  *
  * Copyright (C) 2009 Till Adam <adam@kde.org>
  * Copyright (C) 2009 Flavio Castelli <flavio@castelli.name>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License version 2.1, as published by the Free Software Foundation.
  *
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */


#include "qobjecthelper.h"

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QDebug>

/**
* @brief Class used to convert QObject into QVariant and vivce-versa.
* During these operations only the class attributes defined as properties will
* be considered.
* Properties marked as 'non-stored' will be ignored.
*
* Suppose the declaration of the Person class looks like this:
* \code
* class Person : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(int phoneNumber READ phoneNumber WRITE setPhoneNumber)
  Q_PROPERTY(Gender gender READ gender WRITE setGender)
  Q_PROPERTY(QDate dob READ dob WRITE setDob)
  Q_ENUMS(Gender)

 public:
    Person(QObject* parent = 0);
    ~Person();

    QString name() const;
    void setName(const QString& name);

    int phoneNumber() const;
    void setPhoneNumber(const int  phoneNumber);

    enum Gender {Male, Female};
    void setGender(Gender gender);
    Gender gender() const;

    QDate dob() const;
    void setDob(const QDate& dob);

  private:
    QString m_name;
    int m_phoneNumber;
    Gender m_gender;
    QDate m_dob;
};
\endcode

The following code will serialize an instance of Person to JSON :

\code
    Person person;
    person.setName("Flavio");
    person.setPhoneNumber(123456);
    person.setGender(Person::Male);
    person.setDob(QDate(1982, 7, 12));

    QString  result = QObjectHelper::qobject2json(&person);
    QObjectHelper::json2qobject(result, &person);
    qDebug() << result
    qDebug() << person;
\endcode

The generated output will be:
\code
    "{\n    \"dob\": \"1982-07-12\",\n    \"gender\": 0,\n    \"name\": \"Flavio\",\n    \"phoneNumber\": 123456\n}\n"
    Person({"name":"Flavio","phoneNumber":"123456","gender":"0","dob":"1982-07-12"})
\endcode
*/


class QObjectHelper::QObjectHelperPrivate {
};

QObjectHelper::QObjectHelper()
  : d (new QObjectHelperPrivate)
{
}

QObjectHelper::~QObjectHelper()
{
  delete d;
}



/**
* This method converts a QObject instance into a QVariantMap.
*
* @param object The QObject instance to be converted.
* @param ignoredProperties Properties that won't be converted.
*/
QVariantMap QObjectHelper::qobject2qvariant( const QObject* object,
                              const QStringList& ignoredProperties)
{
    QVariantMap result;
    const QMetaObject *metaobject = object->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();

        if (ignoredProperties.contains(QLatin1String(name)) || (!metaproperty.isReadable()))
            continue;

        QVariant value = object->property(name);
        result[QLatin1String(name)] = value;
    }
    return result;
}


/**
* This method converts a QObject instance into a json string.
*
* @param object The QObject instance to be converted.
* @param ignoredProperties Properties that won't be converted.
*/
QString QObjectHelper::qobject2json(const QObject *object, const QStringList &ignoredProperties)
{
    QString result("");
    QVariantMap variant = QObjectHelper::qobject2qvariant(object, ignoredProperties);
    QJsonDocument doc(QJsonObject::fromVariantMap(variant));
    result = QString::fromLocal8Bit(doc.toJson());
    return result;
}


/**
* This method converts a QVariantMap instance into a QObject
*
* @param variant Attributes to assign to the object.
* @param object The QObject instance to update.
*/
void QObjectHelper::qvariant2qobject(const QVariantMap& variant, QObject* object)
{
    const QMetaObject *metaobject = object->metaObject();

    QVariantMap::const_iterator iter;
    for (iter = variant.constBegin(); iter != variant.constEnd(); ++iter) {
        int pIdx = metaobject->indexOfProperty(iter.key().toLatin1());

        if (pIdx < 0) {
            continue;
        }

        QMetaProperty metaproperty = metaobject->property(pIdx);
        QVariant::Type type = metaproperty.type();
        QVariant v(iter.value());
        if (v.canConvert(type)) {
            v.convert(type);
            metaproperty.write(object, v);
        }else if (QString(QLatin1String("QVariant")).compare(QLatin1String(metaproperty.typeName())) == 0) {
            metaproperty.write(object, v);
        }
    }
}


/**
* This method converts a json string instance into a QObject
*
* @param variant Attributes to assign to the object.
* @param object The QObject instance to update.
*/
void QObjectHelper::json2qobject(const QString &json, QObject *object)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toLocal8Bit(), &error);
    if (error.error == QJsonParseError::NoError){
        QObjectHelper::qvariant2qobject(doc.toVariant().toMap(), object);
    }else{
        qDebug() << error.errorString();
    }
}
