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
