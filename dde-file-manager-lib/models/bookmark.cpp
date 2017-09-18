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

#include "bookmark.h"
#include <QIcon>

BookMark::BookMark(const DUrl &url)
    : DFileInfo(({DUrl tmp_url = DUrl::fromBookMarkFile("/");
                          tmp_url.setFragment(url.toLocalFile());
                          tmp_url;}))
{

}

BookMark::BookMark(QDateTime time, const QString &name, const DUrl &url)
    :DFileInfo(url)
{
    m_time = time;
    m_name = name;
}

BookMark::~BookMark()
{

}

QDateTime BookMark::getDateTime()
{
    return m_time;
}

void BookMark::setDateTime(QDateTime time)
{
    m_time = time;
}

void BookMark::setName(const QString &name)
{
    m_name = name;
}

QString BookMark::getName()
{
    return m_name;
}

QString BookMark::getDevcieId() const
{
    return m_devcieId;
}

void BookMark::setDevcieId(const QString &devcieId)
{
    m_devcieId = devcieId;
}
