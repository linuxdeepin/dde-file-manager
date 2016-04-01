#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include "utils/debugobejct.h"
#include <QString>
#include <QDateTime>

class BookMark : public DebugObejct
{
    Q_OBJECT
public:
    explicit BookMark(QObject *parent = 0);
    BookMark(QDateTime time, const QString &name, const QString &url, QObject *parent = 0);
    ~BookMark();
    QDateTime getDateTime();
    QString getUrl();
    void setDateTime(QDateTime time);
    void setUrl(const QString &url);
    void setName(const QString &name);
    QString getName();
signals:

public slots:

private:
    QString m_url;
    QDateTime m_time;
    QString m_name;
};

#endif // BOOKMARK_H
