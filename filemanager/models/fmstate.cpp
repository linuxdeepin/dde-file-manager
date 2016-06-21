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
int FMState::x() const
{
    return m_x;
}

void FMState::setX(int x)
{
    m_x = x;
}
int FMState::y() const
{
    return m_y;
}

void FMState::setY(int y)
{
    m_y = y;
}
int FMState::windowState() const
{
    return m_windowState;
}

void FMState::setWindowState(int windowState)
{
    m_windowState = windowState;
}





