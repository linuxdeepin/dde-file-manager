#ifndef HIGHLEVELFILEUTILS_H
#define HIGHLEVELFILEUTILS_H

#include <QObject>

class HighLevelFileUtils : public QObject
{
    Q_OBJECT
public:
    explicit HighLevelFileUtils(QObject *parent = 0);
    ~HighLevelFileUtils();

    static QString md5(const QString& data);
    static QString dirName(QString path);
    static QString baseName(QString path);


    static bool isFile(const QString& path);
    static bool isDir(const QString& path);
    static bool isLink(const QString& path);
    static bool exists(const QString& path);

    /* Qt4 version of linux readlink(...) */
    static QString readLink( QString path );

    /* Create a directory: mkdir -p */
    /* http://fossies.org/linux/inadyn/libite/makepath.c */
    static int mkpath(const QString& path, mode_t mode = 0755);

    /* Remove a directory and its contents */
    static bool removeDir(const QString& dirName);

    /* Is the path readable */
    static bool isReadable(const QString& path);
    static bool isWritable(const QString& path);

    /* Get the number of children of a dir */
    static qint64 nChildren(const QString& path);

    /* Get the size of a directory and file */
    static qint64 getSize(const QString& path);

    /* Get all the files and subdirs in directory */
    static QStringList recDirWalk(const QString& path);

    /* Convert the numberic size to human readable string */
    static QString formatSize(qint64 size);

signals:

public slots:
};

#endif // HIGHLEVELFILEUTILS_H
