#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{

}

void DFileMenu::setUrls(const QList<QString> &urls)
{
    m_urls = urls;
}

QList<QString> DFileMenu::getUrls()
{
    return m_urls;
}

void DFileMenu::setDir(const QString &dir)
{
    m_dir = dir;
}

QString DFileMenu::getDir() const
{
    return m_dir;
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

