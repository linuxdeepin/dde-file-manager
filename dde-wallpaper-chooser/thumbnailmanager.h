#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    ThumbnailManager();

    void clear();
    bool find(const QString & key, QPixmap * pixmap);
    void remove(const QString & key);
    bool replace(const QString & key, const QPixmap & pixmap);

    static ThumbnailManager * instance();

private:
    QString m_cacheDir;
};

#endif // THUMBNAILMANAGER_H
