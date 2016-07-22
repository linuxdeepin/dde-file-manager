#include "historystack.h"
#include <QDebug>
#include <QFileInfo>

HistoryStack::HistoryStack(int threshold)
{
    m_threshold = threshold;
    m_index = 0;
}

void HistoryStack::insert(DUrl url)
{
    if(m_index < m_threshold)
    {
        if(m_index == m_list.size())
        {
            m_list.append(url);
        }
        else
        {
            DUrlList localList;
            for(int i = 0; i < m_list.size(); i++)
            {
                DUrl text = m_list.at(i);
                localList.append(text);
            }
            m_list.clear();
            m_list.append(localList);
            m_list.append(url);
        }
        m_index++;
    }
    else
    {
        m_list.takeFirst();
        m_list.append(url);
    }
}

DUrl HistoryStack::back()
{
    if(m_index > 1)
    {
        DUrl url;
        while (m_index > 1) {
            m_index--;
            qDebug() << "index = "<< m_index << m_list;
            url = m_list.at(m_index - 1);
            if (url.isLocalFile()){
                QFileInfo fileinfo(url.path());
                if (!fileinfo.exists()){
                    removeAt(m_index - 1);
                    qDebug() << (m_index - 1) << m_list;
                }else{
                    break;
                }
            }else{
                break;
            }
        }

        return url;

//        m_index--;
//        qDebug() << "index = "<< m_index << m_list;

//        DUrl url = m_list.at(m_index - 1);
//        if (url.isLocalFile()){
//            QFileInfo fileinfo(url.path());
//            if (!fileinfo.exists()){
//                removeAt(m_index - 1);
//                qDebug() << (m_index - 1) << m_list;
//                back();
//            }else{
//                qDebug() << (m_index - 1) << url;
//                return url;
//            }
//        }

//        return url;
    }
    return DUrl();
}

DUrl HistoryStack::forward()
{
    if(m_index < m_list.size())
    {

        DUrl url;
        while (m_index < m_list.size()) {
            m_index++;
            qDebug() << "index = "<< m_index << m_list;
            url = m_list.at(m_index - 1);
            if (url.isLocalFile()){
                QFileInfo fileinfo(url.path());
                if (!fileinfo.exists()){
                    removeAt(m_index - 1);
                    qDebug() << (m_index - 1)  << m_list;
                }else{
                   break;
                }
            }else{
                break;
            }
        }
        return url;

//        m_index++;
//        qDebug() << "index = "<< m_index << m_list;
//        DUrl url = m_list.at(m_index - 1);
//        if (url.isLocalFile()){
//            QFileInfo fileinfo(url.path());
//            if (!fileinfo.exists()){
//                qDebug() << (m_index - 1)  << m_list;
//                removeAt(m_index - 1);
//                qDebug() << m_list;
//                forward();
//            }else{
//                return url;
//            }
//        }
//        return url;
    }
    return DUrl();
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

int HistoryStack::size()
{
    return m_list.size();
}

void HistoryStack::removeAt(int i)
{
    if (i >=0 && i< size()){
        m_list.removeAt(i);
    }
}

int HistoryStack::currentIndex()
{
    return m_index;
}
