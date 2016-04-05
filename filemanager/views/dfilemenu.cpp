#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{

}

void DFileMenu::setUrl(const QString &url)
{
    m_url = url;
}

QString DFileMenu::getUrl()
{
    return m_url;
}
