#include "fileeventreceiver.h"

DSC_BEGIN_NAMESPACE

void FileEventReceiver::eventProcess(const dpf::Event &event)
{
    qInfo() << event;
}


DSC_END_NAMESPACE


