/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    QString getDevcieId() const;
    void setDevcieId(const QString &devcieId);

private:
    QDateTime m_time;
    QString m_name;
    QString m_devcieId;

    // AbstractFileInfo interface
};

#endif // BOOKMARK_H
