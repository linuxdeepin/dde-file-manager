#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{
    m_source = Unknow;
}

void DFileMenu::setUrls(const DUrlList &urls)
{
    m_urls = urls;
}

DUrlList DFileMenu::getUrls()
{
    return m_urls;
}

void DFileMenu::setWindowId(int windowId)
{
    m_windowId = windowId;
}

int DFileMenu::getWindowId() const
{
    return m_windowId;
}

void DFileMenu::setMenuSource(DFileMenu::MenuSource source)
{
    m_source = source;
}

DFileMenu::MenuSource DFileMenu::getMenuSource()
{
    return m_source;
}
