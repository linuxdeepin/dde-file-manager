#ifndef FMEVENT_H
#define FMEVENT_H

#include <QString>

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
        Unknow
    };
    FMEvent();
    int windowId = -1;
    EventSource source = Unknow;
    QString dir;

};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const FMEvent &info);
QT_END_NAMESPACE

#endif // FMEVENT_H
