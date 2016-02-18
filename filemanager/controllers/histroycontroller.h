#ifndef HISTROYCONTROLLER_H
#define HISTROYCONTROLLER_H

#include <QObject>

class HistoryRecord;

class HistroyController : public QObject
{
    Q_OBJECT
public:
    explicit HistroyController(QObject *parent = 0);
    ~HistroyController();

    void initData();

signals:

public slots:

private:
    HistoryRecord* m_histroyUrlRecords = NULL;
    HistoryRecord* m_histroySearchKeywordRecords = NULL;
};

#endif // HISTROYCONTROLLER_H
