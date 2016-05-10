#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H

#include <QList>

#include "durl.h"

class HistoryStack
{
public:
    HistoryStack(int threshold);
    void insert(DUrl url);
    DUrl back();
    DUrl forward();
    void setThreshold(int threshold);
    bool isFirst();
    bool isLast();
    int size();
private:
    QList<DUrl> m_list;
    int m_threshold;
    int m_index;
};

#endif // HISTORYSTACK_H
