#include "fmevent.h"

#include <QDebug>

FMEvent::FMEvent()
{

}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const FMEvent &info)
{
    deg << "window id:" << info.windowId << "source:" << info.source << "url" << info.dir;

    return deg;
}
QT_END_NAMESPACE
