#include "bookmark.h"
#include <QIcon>

BookMark::BookMark(const DUrl &url)
    : DFileInfo(({DUrl tmp_url = DUrl::fromBookMarkFile("/");
                          tmp_url.setFragment(url.toLocalFile());
                          tmp_url;}))
{

}

BookMark::BookMark(QDateTime time, const QString &name, const DUrl &url)
    :DFileInfo(url)
{
    m_time = time;
    m_name = name;
}

BookMark::~BookMark()
{

}

QDateTime BookMark::getDateTime()
{
    return m_time;
}

void BookMark::setDateTime(QDateTime time)
{
    m_time = time;
}

void BookMark::setName(const QString &name)
{
    m_name = name;
}

QString BookMark::getName()
{
    return m_name;
}

QString BookMark::getDevcieId() const
{
    return m_devcieId;
}

void BookMark::setDevcieId(const QString &devcieId)
{
    m_devcieId = devcieId;
}
