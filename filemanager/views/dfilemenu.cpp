#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{

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

void DFileMenu::setFileInfo(const AbstractFileInfo *info)
{
    m_fileInfo = info;
}

const AbstractFileInfo *DFileMenu::fileInfo() const
{
    return m_fileInfo;
}

