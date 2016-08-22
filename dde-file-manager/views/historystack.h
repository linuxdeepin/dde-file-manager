#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H

#include <QList>

#include "durl.h"

class HistoryStack
{
public:
    HistoryStack(int threshold);
    void append(DUrl url);
    DUrl back();
    DUrl forward();
    void setThreshold(int threshold);
    bool isFirst();
    bool isLast();
    int size();
    void removeAt(int i);
    int currentIndex();
private:
    QList<DUrl> m_list;
    int m_threshold;
    int m_index;

    friend QDebug operator<<(QDebug beg, const HistoryStack &stack);
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug beg, const HistoryStack &stack);
QT_END_NAMESPACE

#endif // HISTORYSTACK_H
