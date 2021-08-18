#ifndef EVENT_H
#define EVENT_H

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QString>
#include <QVariant>

DPF_BEGIN_NAMESPACE

class EventPrivate;

class Event
{
    EventPrivate * const d_ptr;
public:
    explicit Event();

    virtual ~Event();

    void setTopic(const QString &topic);
    QString topic();

    void setData(const QVariant &data);
    QVariant data();
};

DPF_END_NAMESPACE

#endif // EVENT_H
