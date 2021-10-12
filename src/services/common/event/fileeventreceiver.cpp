#include "fileeventreceiver.h"

DSC_BEGIN_NAMESPACE

namespace fileOperation
{

}

void FileEventReceiver::eventProcess(const dpf::Event &event)
{
    qInfo() << event;
}

DSC_END_NAMESPACE


