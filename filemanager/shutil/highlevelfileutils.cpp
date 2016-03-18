#include "highlevelfileutils.h"
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

HighLevelFileUtils::HighLevelFileUtils(QObject *parent) : QObject(parent)
{

}

HighLevelFileUtils::~HighLevelFileUtils()
{

}

QString HighLevelFileUtils::md5(const QString &data)
{
    return QString::fromLocal8Bit( QCryptographicHash::hash( data.toLocal8Bit(), QCryptographicHash::Md5).toHex());
}

QString HighLevelFileUtils::dirName(QString path)
{
    while( path.contains( "//" ) )
        path = path.replace( "//", "/" );

    if ( path.endsWith( "/" ) )
        path.chop( 1 );

    char *dupPath = strdup( path.toLocal8Bit().data() );
    QString dirPth = QString( dirname( dupPath ) ) + "/";
    free( dupPath );

    return dirPth;
}

QString HighLevelFileUtils::baseName(QString path)
{
    while( path.contains( "//" ) )
        path = path.replace( "//", "/" );

    if ( path.endsWith( "/" ) )
        path.chop( 1 );

    char *dupPath = strdup( path.toLocal8Bit().data() );
    QString basePth = QString( basename( dupPath ) );
    free( dupPath );

    return basePth;
}

bool HighLevelFileUtils::isFile(const QString &path)
{
    struct stat statbuf;
    if (stat(path.toLocal8Bit().data(), &statbuf ) == 0)
        if (S_ISREG(statbuf.st_mode) or S_ISLNK(statbuf.st_mode))
            return true;
        else
            return false;
    else
        return false;
}

bool HighLevelFileUtils::isDir(const QString &path)
{
    struct stat statbuf;
    if (stat(path.toLocal8Bit().data(), &statbuf) == 0)
        if (S_ISDIR( statbuf.st_mode))
            return true;
        else
            return false;
    else
        return false;
}

bool HighLevelFileUtils::isLink(const QString &path)
{
    struct stat statbuf;
    if (lstat(path.toLocal8Bit().data(), &statbuf) == 0)
        if (S_ISLNK(statbuf.st_mode))
            return true;
        else
            return false;
    else
        return false;
}

bool HighLevelFileUtils::exists(const QString &path)
{
    return not access(path.toLocal8Bit().data(), F_OK);
}

QString HighLevelFileUtils::readLink(QString path)
{
    char linkTarget[ 1024 ] = { 0 };
    readlink( path.toLocal8Bit().data(), linkTarget, 1023);
    return QString(linkTarget);
}

int HighLevelFileUtils::mkpath(const QString &path, mode_t mode)
{
    /* Root always exists */
    if ( path == "/" )
        return 0;

    /* If the directory exists, thats okay for us */
    if (exists( path ) )
        return 0;

    mkpath(dirName(path), mode);
    return mkdir(path.toLocal8Bit().data(), mode);
}

bool HighLevelFileUtils::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir( dirName);

    if ( dir.exists( dirName ) ) {
        foreach( QFileInfo info, dir.entryInfoList( QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst ) ) {
            if ( info.isDir() )
                result = removeDir( info.absoluteFilePath() );

            else
                result = QFile::remove( info.absoluteFilePath() );

            if ( !result )
                return result;
        }
        result = dir.rmdir( dirName );
    }

    return result;
}

bool HighLevelFileUtils::isReadable(const QString &path)
{
    if ( isDir( path ) )
        return not access( path.toLocal8Bit().data(), R_OK | X_OK );
    else
        return not access( path.toLocal8Bit().data(), R_OK );
}

bool HighLevelFileUtils::isWritable(const QString &path)
{
    return not access( path.toLocal8Bit().data(), W_OK );
}

qint64 HighLevelFileUtils::nChildren(const QString &path)
{
    qint64 entries = 0;
    struct dirent *ent;
    DIR *dir = opendir( path.toLocal8Bit().data() );

    if ( dir != NULL ) {
        while ( ( ent = readdir( dir ) ) != NULL)
            entries++;

        // Remove entries corresponding to . and ..
        entries -= 2;
        closedir( dir );
    }

    return entries;
}

qint64 HighLevelFileUtils::getSize(const QString &path)
{
    struct stat statbuf;
    if ( stat( path.toLocal8Bit().data(), &statbuf ) != 0 )
        return 0;

    switch( statbuf.st_mode & S_IFMT ) {
        case S_IFREG: {

            return statbuf.st_size;
        }

        case S_IFDIR: {
            DIR* d_fh;
            struct dirent* entry;
            QString longest_name;

            while ( ( d_fh = opendir( path.toLocal8Bit().data() ) ) == NULL ) {
                qWarning() << "Couldn't open directory:" << path;
                return statbuf.st_size;
            }

            quint64 size = statbuf.st_size;

            longest_name = QString( path );
            if ( not longest_name.endsWith( "/" ) )
                longest_name += "/";

            while( ( entry = readdir( d_fh ) ) != NULL ) {

                /* Don't descend up the tree or include the current directory */
                if ( strcmp( entry->d_name, ".." ) != 0 && strcmp( entry->d_name, "." ) != 0 ) {

                    if ( entry->d_type == DT_DIR ) {

                        /* Recurse into that folder */
                        size += getSize( longest_name + entry->d_name );
                    }

                    else {

                        /* Get the size of the current file */
                        size += getSize( longest_name + entry->d_name );
                    }
                }
            }

            closedir( d_fh );
            return size;
        }

        default: {

            /* Return 0 for all other nodes: chr, blk, lnk, symlink etc */
            return 0;
        }
    }

    /* Should never come till here */
    return 0;
}

QStringList HighLevelFileUtils::recDirWalk(const QString &path)
{
    QStringList fileList;

    if ( not QFileInfo( path ).exists() )
        return QStringList();

    QDirIterator it( path, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories );
    while ( it.hasNext() ) {
        it.next();
        if( it.fileInfo().isDir() ) {
            fileList.append( it.fileInfo().filePath() );
        }

        else {
            fileList.append( it.fileInfo().filePath() );
        }
    }

    return fileList;
}

QString HighLevelFileUtils::formatSize(qint64 size)
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( size >= tb )
        total = QString( "%1 TiB" ).arg( QString::number( qreal( size ) / tb, 'f', 3 ) );
    else if ( size >= gb )
        total = QString( "%1 GiB" ).arg( QString::number( qreal( size ) / gb, 'f', 2 ) );
    else if ( size >= mb )
        total = QString( "%1 MiB" ).arg( QString::number( qreal( size ) / mb, 'f', 1 ) );
    else if ( size >= kb )
        total = QString( "%1 KiB" ).arg( QString::number( qreal( size ) / kb,'f',1 ) );
    else
        total = QString( "%1 byte%2" ).arg( size ).arg( size > 1 ? "s": "" );

    return total;
}

