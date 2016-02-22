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
    void initConnect();

signals:

public slots:
    void appendHistroyUrlRecords(const QUrl& url);

private:
    HistoryRecord* m_histroyUrlRecords = NULL;
    HistoryRecord* m_histroySearchKeywordRecords = NULL;
};

#endif // HISTROYCONTROLLER_H
