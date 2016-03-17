#include "searchhistory.h"

SearchHistory::SearchHistory(QObject *parent) : DebugObejct(parent)
{

}

SearchHistory::SearchHistory(QDateTime time, QString keyword, QObject *parent)
{
    m_time = time;
    m_keyword = keyword;
}

QDateTime SearchHistory::getDateTime()
{

}

QString SearchHistory::getKeyword()
{

}

void SearchHistory::setDateTime(QDateTime time)
{

}

void SearchHistory::setKeyword(QString keyword)
{

}

SearchHistory::~SearchHistory()
{

}
