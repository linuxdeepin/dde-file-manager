#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QFile>

#define COPY_BUF_SIZE ( 64 * 1024 )

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO(QObject *parent = 0);
    explicit FileIO(const QStringList& files, const QString& target, QObject *parent = 0);
    ~FileIO();

    enum IOMode{
        Copy,
        Move,
        AllCopy
    };

    void init();

    QStringList sources();// Return the source list
    void setSources(const QStringList& files);
    QString target(); //Return the target
    void setTarget(const QString& target);// Set the target - Where to Copy/Move/ACopy


    QString jobId();// ID - Identity of this IO process
    int result(); // What is the result of the IO
    QStringList errors(); // The list of nodes which could not be copied/moved/archived
    IOMode mode(); // return job mode

signals:
    void IOComplete();

public slots:
    void performIO(); // Start the IO
    void preIO(); // Things to be done before IO begins like computing sizes
    void cancel(); // Cancel the IO Operation
    void pause();// Pause the IO Operation
    void resume();// Resume the paused IO
    void getDirSize(const QString& path);// Compute the size of a directory
    void copyFile(const QString& file); // Copy a file
    void copyDir(const QString& path); // Copy a folder
    void mkpath(const QString& path, QFile::Permissions permission);// Create a directory tree - A directory with sub-directory, sub-sub-directory, and so on

private:
    quint64 m_totalSize; // Total bytes to be copied
    quint64 m_copiedSize;// Bytes copied so far
    quint64 m_fTotalBytes;// Size of the current file
    quint64 m_fWritten;// Bytes copied of the current file
    QString m_ioTarget;// The currently being written file

    QStringList m_errorNodes;
    QStringList m_sourceList;
    QString m_targetDir;

    QString m_jobId;

    bool m_isCanceled;
    bool m_isPaused;

    IOMode m_mode;
};

#endif // FILEIO_H
