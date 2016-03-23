#include "historystack.h"
#include <QDebug>

HistoryStack::HistoryStack(int threshold)
{
    m_threshold = threshold;
    m_index = 0;
}

void HistoryStack::insert(QString path)
{
    if(m_index < m_threshold)
    {
        if(m_index == m_list.size())
        {
            m_list.append(path);
            qDebug() <<"append";
        }
        else
        {
            QList<QString> localList;
            for(int i = 0; i <= m_index; i++)
            {
                QString text = m_list.at(i);
                localList.append(text);
            }
            m_list.clear();
            m_list.append(localList);
            m_list.append(path);
        }
        m_index++;
    }
    else
    {
        m_list.takeFirst();
        m_list.append(path);
    }
}

QString HistoryStack::back()
{
    if(m_index > 1)
    {
        m_index--;
        qDebug() << "index = "<<m_index;
        return m_list.at(m_index - 1);
    }
    return "";
}

QString HistoryStack::forward()
{
    if(m_index < m_list.size())
    {
        m_index++;
        return m_list.at(m_index - 1);
    }
    return "";
}

void HistoryStack::setThreshold(int threshold)
{
    m_threshold = threshold;
}

bool HistoryStack::isFirst()
{
    return m_index == 1;
}

bool HistoryStack::isLast()
{
    return m_index == m_list.size();
}
