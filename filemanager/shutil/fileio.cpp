#include "fileio.h"
#include "../shutil/highlevelfileutils.h"
#include <QDir>
#include <sys/statvfs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <dirent.h>
#include "../shutil/mimeutils.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>

FileIO::FileIO(QObject *parent) : QObject(parent)
{
    init();
}

FileIO::FileIO(const QStringList &files, const QString &target, QObject *parent):
    QObject(parent)
{
    init();
    setSources(files);
    setTarget(target);
}

FileIO::~FileIO()
{

}

void FileIO::init()
{
    m_totalSize = 0;
    m_copiedSize = 0;
    m_fTotalBytes = 0;
    m_fWritten = 0;


    m_ioTarget = QString();

    m_errorNodes.clear();
    m_sourceList.clear();
    m_targetDir = QString();

    m_isCanceled = false;
    m_isPaused = false;

    m_mode = Copy;
    m_jobId = HighLevelFileUtils::md5(QDateTime::currentDateTime().toString( Qt::ISODate ) );
}

QStringList FileIO::sources()
{
    return m_sourceList;
}

void FileIO::setSources(const QStringList &files)
{
    m_sourceList = files;
}

QString FileIO::target()
{
    return m_targetDir;
}

void FileIO::setTarget(const QString &target)
{
    m_targetDir = target.endsWith("/") ? target : target + "/";
    m_jobId = HighLevelFileUtils::md5(m_targetDir + QDateTime::currentDateTime().toString(Qt::ISODate));
}

QString FileIO::jobId()
{
    return m_jobId;
}

int FileIO::result()
{
    if (m_errorNodes.count())
        return 1;
    else
        return 0;
}

QStringList FileIO::errors()
{
    return m_errorNodes;
}

FileIO::IOMode FileIO::mode()
{
    return m_mode;
}

void FileIO::performIO()
{
    preIO();

    QString currentDir = QDir::current().absolutePath();

    foreach( QString node, m_sourceList ) {
        if ( m_isCanceled )
            break;

        while ( m_isPaused ) {
            if ( m_isCanceled )
                break;

            usleep( 100 );
            qApp->processEvents();
        }

        QString srcPath = HighLevelFileUtils::dirName( node );
        QString srcBase = HighLevelFileUtils::baseName( node );

        chdir( srcPath.toLocal8Bit().data() );
        if ( HighLevelFileUtils::isDir( node ) )
            copyDir( srcBase );

        else
            copyFile( srcBase );

        chdir( currentDir.toLocal8Bit().data() );
    }

    emit IOComplete();
}

void FileIO::preIO()
{
    QString currentDir = QDir::current().absolutePath();

    foreach( QString node, m_sourceList ) {
        QString srcPath = HighLevelFileUtils::dirName( node );
        QString srcBase = HighLevelFileUtils::baseName( node );

        chdir( srcPath.toLocal8Bit().data() );
        if ( HighLevelFileUtils::isDir( node ) ) {
            /*
                *
                * This is a top level directory. This won't be created in @f getDirSize( ... )
                * We need to create this directory and get its size.
                *
            */

            /* If we are moving and its an intra-device move then mkpath won't be necessary */
            struct stat iStat, oStat;
            stat( node.toLocal8Bit().data(), &iStat );
            stat( m_targetDir.toLocal8Bit().data(), &oStat );

            if ( ( iStat.st_dev == oStat.st_dev ) and ( Move == m_mode ) )
                continue;

            else {
                mkpath( srcBase, QFile::permissions( srcBase ) );
                getDirSize( srcBase );
            }
        }

        else {
            // This is a file. Just get its size
            m_totalSize += HighLevelFileUtils::getSize( srcBase );
        }

        chdir( currentDir.toLocal8Bit().data() );
    }
}

void FileIO::cancel()
{
    m_isCanceled = true;
}

void FileIO::pause()
{
    m_isPaused = true;
}

void FileIO::resume()
{
    m_isPaused = false;
}

void FileIO::getDirSize(const QString &path)
{
    DIR* d_fh;
    struct dirent* entry;
    QString longest_name;

    while( ( d_fh = opendir( path.toLocal8Bit().data() ) ) == NULL ) {
        qWarning() << "Couldn't open directory:" << path;
        return;
    }

    longest_name = QString( path );
    if ( not longest_name.endsWith( "/" ) )
        longest_name += "/";

    while( ( entry = readdir( d_fh ) ) != NULL ) {

        /* Don't descend up the tree or include the current directory */
        if ( strcmp( entry->d_name, ".." ) != 0 && strcmp( entry->d_name, "." ) != 0 ) {

            if ( entry->d_type == DT_DIR ) {

                /*
                    *
                    * Prepend the current directory and recurse
                    * We also need to create this directory in
                    * @v targetDir
                    *
                */
                mkpath( longest_name + entry->d_name, QFile::permissions( longest_name + entry->d_name ) );
                getDirSize( longest_name + entry->d_name );
            }
            else {

                /* Copy the current file */
                m_totalSize += HighLevelFileUtils::getSize(longest_name + entry->d_name );
            }
        }
    }

    closedir( d_fh );
}

void FileIO::copyFile(const QString &srcFile)
{
    m_ioTarget = m_targetDir + srcFile;

    if ( not HighLevelFileUtils::isReadable( srcFile ) ) {
        qDebug() << "Unreadable file: " + srcFile;
        m_errorNodes << srcFile;
        return;
    }

    if ( not HighLevelFileUtils::isWritable( HighLevelFileUtils::dirName( m_ioTarget ) ) ) {
        qDebug() << m_ioTarget + " not writable!!!";
        m_errorNodes << srcFile;
        // return;
    }

    if ( HighLevelFileUtils::exists( m_ioTarget ) ) {
//        QString suffix = mimeDb.suffixForFileName( srcFile );
        QString suffix =
        m_ioTarget = ( suffix.length() ? m_ioTarget.left( m_ioTarget.length() - ( 1 + suffix.length() ) ) + " - Copy." + suffix : m_ioTarget + " - Copy" );
    }

    struct stat iStat, oStat;
    stat( srcFile.toLocal8Bit().data(), &iStat );
    stat( m_targetDir.toLocal8Bit().data(), &oStat );

    /* If the operation is intra-device operation and its a move, then we can simply rename the file */
    if ( ( iStat.st_dev == oStat.st_dev ) and ( Move == m_mode ) ) {
        QFile::rename( srcFile, m_ioTarget );
        return;
    }

    int iFileFD = open( srcFile.toLocal8Bit().data(), O_RDONLY );
    int oFileFD = open( m_ioTarget.toLocal8Bit().data(), O_WRONLY | O_CREAT, iStat.st_mode );

    m_fTotalBytes = iStat.st_size;
    m_fWritten = 0;

    ssize_t inBytes = 0;
    ssize_t bytesWritten = 0;
    char block[ COPY_BUF_SIZE ];

    while ( ( inBytes = read( iFileFD, block, sizeof( block ) ) ) > 0 ) {
        if ( m_isCanceled ) {
            close( iFileFD );
            close( oFileFD );

            return;
        }

        while ( m_isPaused ) {
            if ( m_isCanceled ){
                close( iFileFD );
                close( oFileFD );

                return;
            }

            usleep( 100 );
            qApp->processEvents();
        }

        bytesWritten = write( oFileFD, block, inBytes );

        if ( bytesWritten != inBytes ) {
            qDebug() << "Error writing to file:" << m_ioTarget;
            qDebug() << "[Error]:" << strerror( errno );
            m_errorNodes << srcFile;
            break;
        }

        m_fWritten += bytesWritten;
        m_copiedSize += bytesWritten;
        qApp->processEvents();
    }

    close( iFileFD );
    close( oFileFD );

    /* If read(...) resulted in an error */
    if ( inBytes == -1 ) {
        qDebug() << "Error copying file:" << srcFile;
        qDebug() << "[Error]:" << strerror( errno );
        m_errorNodes << srcFile;
    }

    if ( m_fWritten != quint64( iStat.st_size ) )
        m_errorNodes << srcFile;

    if ( m_mode == Move ) {
        if ( not m_errorNodes.contains( srcFile ) )
            unlink( srcFile.toLocal8Bit().data() );
    }
}

void FileIO::copyDir(const QString &path)
{
    /* If the operation is intra-device operation and its a move, then we can simply rename the file */
    struct stat iStat, oStat;
    stat( path.toLocal8Bit().data(), &iStat );
    stat( m_targetDir.toLocal8Bit().data(), &oStat );

    if ( ( iStat.st_dev == oStat.st_dev ) and ( Move == m_mode ) ) {
        /*
            *
            * If the rename fails, then the target may be existing and is not empty
            * In such a case, we must perform a normal copy
            *
        */
        if ( not rename( path.toLocal8Bit().data(), ( m_targetDir + HighLevelFileUtils::baseName( path ) ).toLocal8Bit().data() ) )
            return;
    }

    DIR* d_fh;
    struct dirent* entry;
    QString longest_name;

    if ( m_isCanceled )
        return;

    while( ( d_fh = opendir( path.toLocal8Bit().data() ) ) == NULL ) {
        qWarning() << "Couldn't open directory:" << path;
        return;
    }

    longest_name = QString( path );
    if ( not longest_name.endsWith( "/" ) )
        longest_name += "/";

    while( ( entry = readdir( d_fh ) ) != NULL ) {

        /* Don't descend up the tree or include the current directory */
        if ( strcmp( entry->d_name, ".." ) != 0 && strcmp( entry->d_name, "." ) != 0 ) {

            /* If it's a directory print it's name and recurse into it */
            if ( entry->d_type == DT_DIR ) {

                /* Prepend the current directory and recurse */
                copyDir( longest_name + entry->d_name );
            }
            else {

                /* Copy the current file */
                copyFile( longest_name + entry->d_name );
            }
        }
    }

    closedir( d_fh );

    if ( m_mode == Move )
        rmdir( path.toLocal8Bit().data() );
}

void FileIO::mkpath(const QString& path, QFileDevice::Permissions permission)
{
    QString currentDir = QDir::current().absolutePath();
    chdir(m_targetDir.toLocal8Bit().data());
    QString __path;
    foreach( QString pathBit, path.split( "/", QString::SkipEmptyParts ) ) {
        __path += pathBit + "/";
        if ( not HighLevelFileUtils::exists( __path ) ) {
            mkdir( __path.toLocal8Bit().data(), S_IRWXU );
            QFile::setPermissions( __path, permission );
        }
    }
    chdir(currentDir.toLocal8Bit().data());
}


