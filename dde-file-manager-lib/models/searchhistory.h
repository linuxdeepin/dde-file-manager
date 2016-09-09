#ifndef SEARCHHISTORY_H
#define SEARCHHISTORY_H

#include <QObject>
#include <QDateTime>
#include "debugobejct.h"

class SearchHistory : public DebugObejct
{
    Q_OBJECT
public:
    explicit SearchHistory(QObject *parent = 0);
    SearchHistory(QDateTime time, QString keyword, QObject *parent = 0);
    QDateTime getDateTime();
    QString getKeyword();
    void setDateTime(QDateTime time);
    void setKeyword(QString keyword);
    ~SearchHistory();
private:
    QDateTime m_time;
    QString m_keyword;
signals:

public slots:
};

#endif // SEARCHHISTORY_H
