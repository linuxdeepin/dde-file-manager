#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H


#include <QObject>

class TrashJobController;
class MoveJobController;
class CopyJobController;
class RenameJobController;
class DeleteJobController;
class FileController;
class FileMonitor;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

    void initConnect();

    TrashJobController* getTrashJobController() const;
    MoveJobController* getMoveJobController() const;
    CopyJobController* getCopyJobController() const;
    RenameJobController* getRenameJobController() const;
    DeleteJobController* getDeleteJobController() const;
    FileController* getFileController() const;
    FileMonitor *getFileMonitor() const;

private:
    TrashJobController* m_trashJobController;
    MoveJobController* m_moveJobController;
    CopyJobController* m_copyJobController;
    RenameJobController* m_renameJobController;
    DeleteJobController* m_deleteJobController;
    FileController* m_fileController;
    FileMonitor *m_fileMonitor;
};

#endif // APPCONTROLLER_H
