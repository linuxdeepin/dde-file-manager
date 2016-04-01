#include "searchhistory.h"

SearchHistory::SearchHistory(QObject *parent) : DebugObejct(parent)
{

}

SearchHistory::SearchHistory(QDateTime time, QString keyword, QObject *parent)
    : DebugObejct(parent)
{
    m_time = time;
    m_keyword = keyword;
}

QDateTime SearchHistory::getDateTime()
{
    return m_time;
}

QString SearchHistory::getKeyword()
{
    return m_keyword;
}

void SearchHistory::setDateTime(QDateTime time)
{
    m_time = time;
}

void SearchHistory::setKeyword(QString keyword)
{
    m_keyword = keyword;
}

SearchHistory::~SearchHistory()
{

}
