#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H


#include <QObject>

class FileController;
class FileMonitor;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

    void initConnect();
    FileController* getFileController() const;
    FileMonitor* getFileMonitor() const;

private:
    FileController* m_fileController;
    FileMonitor* m_fileMonitor;
};

#endif // APPCONTROLLER_H
