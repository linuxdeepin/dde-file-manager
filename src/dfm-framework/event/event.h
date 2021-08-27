#ifndef EVENT_H
#define EVENT_H

#include "dfm-framework/dfm_framework_global.h"

#include <QString>
#include <QVariant>
#include <QSharedData>

DPF_BEGIN_NAMESPACE

class EventPrivate;

/**
 * @brief The Event class
 *  事件数据源，只能当做类使用不可继承
 *  禁止被继承
 */
class Event final
{
    EventPrivate * const d_ptr;
    friend Q_CORE_EXPORT QDebug operator <<(QDebug, const Event &);

public:
    explicit Event();
    Event(const Event& event);

    virtual ~Event();

    void setTopic(const QString &topic);
    QString topic() const;

    void setData(const QVariant &data);
    QVariant data() const;

    void setProperty(const QString& key, const QVariant value);
    QVariant property(const QString &key) const;
};

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator <<(QDebug, const DPF_NAMESPACE::Event &);
#endif //QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

DPF_END_NAMESPACE

#endif // EVENT_H
