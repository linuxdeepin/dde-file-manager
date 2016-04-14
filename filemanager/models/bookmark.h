#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include "utils/debugobejct.h"
#include <QString>
#include <QDateTime>
#include "abstractfileinfo.h"

class BookMark : public AbstractFileInfo
{
public:
    BookMark();
    BookMark(BookMark * bookmark);
    BookMark(const QString &url);
    BookMark(QDateTime time, const QString &name, const QString &url);
    ~BookMark();
    QDateTime getDateTime();
    QString getUrl();
    void setDateTime(QDateTime time);
    void setUrl(const QString &url);
    void setName(const QString &name);
    QString getName();

private:
    QString m_url;
    QDateTime m_time;
    QString m_name;

    // AbstractFileInfo interface
public:
    bool isCanRename() const;
    bool isDir() const;
    QIcon fileIcon() const;
};

#endif // BOOKMARK_H
