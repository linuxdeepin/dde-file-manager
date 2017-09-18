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

#ifndef DDIRITERATOR_H
#define DDIRITERATOR_H

#include "dabstractfileinfo.h"

class DDirIterator
{
public:
    virtual ~DDirIterator() {}

    virtual DUrl next() = 0;
    virtual bool hasNext() const = 0;
    virtual void close() {}

    virtual QString fileName() const = 0;
    virtual QString filePath() const = 0;
    virtual const DAbstractFileInfoPointer fileInfo() const = 0;
    virtual QString path() const = 0;

    virtual bool enableIteratorByKeyword(const QString &keyword) {Q_UNUSED(keyword); return false;}
};

typedef QSharedPointer<DDirIterator> DDirIteratorPointer;

Q_DECLARE_METATYPE(DDirIteratorPointer)
#endif // DDIRITERATOR_H
