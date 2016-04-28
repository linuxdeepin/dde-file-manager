#include "fmstate.h"

FMState::FMState(QObject *parent) : DebugObejct(parent)
{

}

FMState::~FMState()
{

}
int FMState::viewMode() const
{
    return m_viewMode;
}

void FMState::setViewMode(int viewMode)
{
    m_viewMode = viewMode;
}


int FMState::width() const
{
    return m_width;
}

void FMState::setWidth(int width)
{
    m_width = width;
}
int FMState::height() const
{
    return m_height;
}

void FMState::setHeight(int height)
{
    m_height = height;
}


