#include "dfmevent.h"

#include <QDebug>

DFMEvent::DFMEvent(int wId, DFMEvent::EventSource source, const DUrl &fileUrl)
    : data(new FMEventData)
{
    data->windowId = wId;
    data->source = source;
    data->fileUrl = fileUrl;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info)
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
