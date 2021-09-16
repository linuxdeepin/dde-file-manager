#include "laucheventreceiver.h"

LauchEventReceiver::LauchEventReceiver()
{

}

void LauchEventReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data().toString() == LauchEventTypes::DATA_OPEN_BY_APP)
        openByApp(event);
}

void LauchEventReceiver::openByApp(const dpf::Event &event)
{
    qInfo() << event;
}
