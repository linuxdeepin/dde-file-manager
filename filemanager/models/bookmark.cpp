#include "bookmark.h"

BookMark::BookMark(QObject *parent) : DebugObejct(parent)
{

}

BookMark::BookMark(QDateTime time, const QString &name, const QString &url, QObject *parent)
    :DebugObejct(parent)
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

