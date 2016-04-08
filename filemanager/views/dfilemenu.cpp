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

QString DFileMenu::getDir()
{
    return m_dir;
}

