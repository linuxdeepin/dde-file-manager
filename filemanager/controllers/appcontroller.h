#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H


#include <QObject>

class TrashJobController;
class MoveJobController;
class CopyJobController;
class RenameJobController;
class DeleteJobController;
class FileController;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

    void initConnect();
    TrashJobController* getTrashJobController();
    MoveJobController* getMoveJobController();
    CopyJobController* getCopyJobController();
    RenameJobController* getRenameJobController();
    DeleteJobController* getDeleteJobController();
    FileController* getFileController();
signals:

public slots:

private:
    TrashJobController* m_trashJobController;
    MoveJobController* m_moveJobController;
    CopyJobController* m_copyJobController;
    RenameJobController* m_renameJobController;
    DeleteJobController* m_deleteJobController;
    FileController* m_fileController;
};

#endif // APPCONTROLLER_H
