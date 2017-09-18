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

#include "thumbnailmanager.h"

#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QStandardPaths>
#include <QApplication>

ThumbnailManager::ThumbnailManager() :
    QObject(NULL)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheDir = cacheDir + QDir::separator() + qApp->applicationVersion();

    QDir::root().mkpath(m_cacheDir);
}

Q_GLOBAL_STATIC(ThumbnailManager, ThumbnailManagerInstance)

void ThumbnailManager::clear()
{
    QDir dir(m_cacheDir);
    dir.removeRecursively();
    QDir::root().mkpath(m_cacheDir);
}

bool ThumbnailManager::find(const QString &key, QPixmap *pixmap)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        *pixmap = QPixmap(file);
        return true;
    } else {
        return false;
    }
}

void ThumbnailManager::remove(const QString &key)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        QFile(file).remove();
    }
}

bool ThumbnailManager::replace(const QString &key, const QPixmap &pixmap)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        QFile(file).remove();
    }
    pixmap.save(file);

    return true;
}

ThumbnailManager *ThumbnailManager::instance()
{
   return ThumbnailManagerInstance;
}
