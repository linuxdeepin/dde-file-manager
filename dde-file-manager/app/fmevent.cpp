#include "fmevent.h"

#include <QDebug>

FMEvent::FMEvent(int wId, FMEvent::EventSource source, const DUrl &fileUrl)
    : data(new FMEventData)
{
    data->windowId = wId;
    data->source = source;
    data->fileUrl = fileUrl;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const FMEvent &info)
{
    deg << "window id:" << info.windowId() << "source:" << info.source() << "url" << info.fileUrl();
    deg << "urlList{";
    foreach (DUrl url, info.fileUrlList()) {
        deg << url;
    }
    deg << "}";
    return deg;
}
QT_END_NAMESPACE
