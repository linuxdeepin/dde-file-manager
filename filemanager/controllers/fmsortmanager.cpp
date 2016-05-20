#include "fmsortmanager.h"

fmsortmanager::fmsortmanager(QObject *parent) :
    QObject(parent),
    BaseManager()
{

}

fmsortmanager::~fmsortmanager()
{

}

void fmsortmanager::load()
{

}

void fmsortmanager::save()
{

}

void fmsortmanager::saveLastSortOrder(const DUrl &url, int role)
{
    Q_UNUSED(url);
    Q_UNUSED(role);
}

