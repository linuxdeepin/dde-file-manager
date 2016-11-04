#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include "debugobejct.h"
#include <QString>
#include <QDateTime>
#include "dfileinfo.h"

class BookMark;
typedef QExplicitlySharedDataPointer<BookMark> BookMarkPointer;

class BookMark : public DFileInfo
{
public:
    BookMark(const DUrl &url);
    BookMark(QDateTime time, const QString &name, const DUrl &url);
    ~BookMark();
    QDateTime getDateTime();
    inline DUrl getUrl()
    {return fileUrl();}
    void setDateTime(QDateTime time);
    void setName(const QString &name);
    QString getName();

private:
    QDateTime m_time;
    QString m_name;

    // AbstractFileInfo interface
public:
    QIcon fileIcon() const Q_DECL_OVERRIDE;
};

#endif // BOOKMARK_H
