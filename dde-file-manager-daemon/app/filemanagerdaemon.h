#ifndef FILEMANAGERDAEMON_H
#define FILEMANAGERDAEMON_H

#include <QObject>

class AppController;

class FileManagerDaemon : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerDaemon(QObject *parent = 0);
    ~FileManagerDaemon();

    void initControllers();
    void initConnect();

signals:

public slots:

private:
    AppController* m_appController = NULL;
};

#endif // FILEMANAGERDAEMON_H
