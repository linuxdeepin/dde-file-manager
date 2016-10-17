#ifndef FMEVENT_H
#define FMEVENT_H

#include <QString>
#include <QSharedData>
#include <QMetaType>

#include "durl.h"

class DFMEvent
{
public:
    enum EventSource {
        CrumbButton,
        LeftSideBar,
        BackAndForwardButton,
        UpButton,
        FileView,
        SearchBar,
        Menu,
        Unknow
    };

    class FMEventData : public QSharedData
    {
    private:
        int windowId = -1;
        EventSource source = Unknow;
        EventSource parentSource = Unknow;
        DUrl fileUrl;
        DUrlList fileUrlList;

        friend class DFMEvent;
        int bookMarkIndex = -1;
    };

    explicit DFMEvent(int wId = -1, EventSource source = Unknow, const DUrl &fileUrl = DUrl());

    inline DFMEvent(const DFMEvent &other)
        : data(other.data)
    {}

    inline DFMEvent(DFMEvent &&other) Q_DECL_NOEXCEPT
    {qSwap(data, other.data);}

    inline DFMEvent &operator =(const DFMEvent &other)
    {data = other.data; return *this;}

    inline DFMEvent &operator =(DFMEvent &&other) Q_DECL_NOEXCEPT
    {qSwap(data, other.data); return *this;}

    inline DFMEvent &operator <<(EventSource source)
    {
        data->parentSource = data->source;
        data->source = source;
        return *this;
    }

    inline DFMEvent &operator <<(int wId)
    {data->windowId = wId; return *this;}

    inline DFMEvent &operator <<(const DUrl &fileUrl)
    {data->fileUrl = fileUrl; return *this;}

    inline DFMEvent &operator <<(const DUrlList &list)
    {data->fileUrlList = list; return *this;}

    inline int windowId() const
    {return data->windowId;}

    inline EventSource source() const
    {return data->source;}

    inline EventSource parentSource() const
    {return data->parentSource;}

    inline const DUrl &fileUrl() const
    {return data->fileUrl;}

    inline const DUrlList &fileUrlList() const
    {return data->fileUrlList;}

    inline const int &bookmarkIndex() const
    {return data->bookMarkIndex;}

    inline void setBookmarkIndex(const int& index)
    {data->bookMarkIndex = index;}

private:
    QSharedDataPointer<FMEventData> data;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info);
QT_END_NAMESPACE

#endif // FMEVENT_H
