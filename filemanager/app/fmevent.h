#ifndef FMEVENT_H
#define FMEVENT_H

#include <QString>
#include <QSharedData>
#include <QMetaType>

#include "durl.h"

class FMEvent
{
public:
    enum EventSource {
        CrumbButton,
        LeftSideBar,
        BackAndForwardButton,
        UpButton,
        FileView,
        SearchLine,
        Menu,
        Unknow
    };

    class FMEventData : public QSharedData
    {
    private:
        int windowId = -1;
        EventSource source = Unknow;
        DUrl fileUrl;

        friend class FMEvent;
    };

    FMEvent(int wId = -1, EventSource source = Unknow, const DUrl &fileUrl = DUrl());

    inline FMEvent(const FMEvent &other)
        : data(other.data)
    {}

    inline FMEvent(FMEvent &&other) Q_DECL_NOEXCEPT
    {qSwap(data, other.data);}

    inline FMEvent &operator =(const FMEvent &other)
    {data = other.data; return *this;}

    inline FMEvent &operator =(FMEvent &&other) Q_DECL_NOEXCEPT
    {qSwap(data, other.data); return *this;}

    inline FMEvent &operator =(EventSource source)
    {data->source = source; return *this;}

    inline FMEvent &operator =(int wId)
    {data->windowId = wId; return *this;}

    inline FMEvent &operator =(const DUrl &fileUrl)
    {data->fileUrl = fileUrl; return *this;}

    inline int windowId() const
    {return data->windowId;}

    inline EventSource source() const
    {return data->source;}

    inline const DUrl &fileUrl() const
    {return data->fileUrl;}

private:
    QSharedDataPointer<FMEventData> data;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const FMEvent &info);
QT_END_NAMESPACE

#endif // FMEVENT_H
