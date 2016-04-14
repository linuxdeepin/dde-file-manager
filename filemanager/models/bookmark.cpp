#include "bookmark.h"
#include <QIcon>

BookMark::BookMark()
{

}

BookMark::BookMark(BookMark *bookmark)
    : AbstractFileInfo(bookmark->getUrl())
{
    m_url = bookmark->getUrl();
    m_time = bookmark->getDateTime();
    m_name = bookmark->getName();
}

BookMark::BookMark(const QString &url)
    : AbstractFileInfo(url)
{
    m_url = url;
}

BookMark::BookMark(QDateTime time, const QString &name, const QString &url)
    :AbstractFileInfo(url)
{
    m_url = url;
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

QString BookMark::getUrl()
{
    return m_url;
}

void BookMark::setDateTime(QDateTime time)
{
    m_time = time;
}

void BookMark::setUrl(const QString &url)
{
    m_url = url;
}

void BookMark::setName(const QString &name)
{
    m_name = name;
}

QString BookMark::getName()
{
    return m_name;
}

bool BookMark::isCanRename() const
{
    return false;
}

bool BookMark::isDir() const
{
    return true;
}

QIcon BookMark::fileIcon() const
{
    return QIcon(":/icons/images/icons/bookmarks_normal_22px.svg");
}

