#include "fileeventreceiver.h"

DSC_BEGIN_NAMESPACE

DPF_EVENT_HANDLER(FileEventReceiver,
                  FileEventTypes::TOPIC_FILE_EVENT);

FileEventReceiver::FileEventReceiver()
{

}

void FileEventReceiver::eventProcess(const dpf::Event &event)
{
    qInfo() << event;
}


DSC_END_NAMESPACE


