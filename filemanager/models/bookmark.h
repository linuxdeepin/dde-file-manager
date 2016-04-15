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
    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
};

#endif // BOOKMARK_H
