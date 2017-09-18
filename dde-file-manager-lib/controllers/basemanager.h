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

#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QByteArray>

class BaseManager
{
public:
    explicit BaseManager();
    ~BaseManager();

    virtual void load();
    virtual void save();

    static QString getCachePath(const QString& fileBaseName);
    static QString getConfigPath(const QString& fileBaseName);
    static void writeCacheToFile(const QString &path, const QString &content);
    static QString readCacheFromFile(const QString &path);

};

#endif // BASEMANAGER_H
