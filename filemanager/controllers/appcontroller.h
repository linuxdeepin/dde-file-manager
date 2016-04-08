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

    FileMonitor* getFileMonitor() const;

private:
    FileMonitor* m_fileMonitor;
};

#endif // APPCONTROLLER_H
