#ifndef QOBJECTHELPER_H
#define QOBJECTHELPER_H

#include <QtCore/QLatin1String>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE


class QObjectHelper {
    public:
      QObjectHelper();
      ~QObjectHelper();


    static QVariantMap qobject2qvariant( const QObject* object,
                                  const QStringList& ignoredProperties = QStringList(QString(QLatin1String("objectName"))));


    static QString qobject2json( const QObject* object,
                                  const QStringList& ignoredProperties = QStringList(QString(QLatin1String("objectName"))));


    static void qvariant2qobject(const QVariantMap& variant, QObject* object);

    static void json2qobject(const QString& json, QObject* object);

    private:
      Q_DISABLE_COPY(QObjectHelper)
      class QObjectHelperPrivate;
      QObjectHelperPrivate* const d;
};


#endif // QOBJECTHELPER_H
