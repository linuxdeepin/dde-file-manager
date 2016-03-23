#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H

#include <QList>

class HistoryStack
{
public:
    HistoryStack(int threshold);
    void insert(QString path);
    QString back();
    QString forward();
    void setThreshold(int threshold);
    bool isFirst();
    bool isLast();
private:
    QList<QString> m_list;
    int m_threshold;
    int m_index;
};

#endif // HISTORYSTACK_H
