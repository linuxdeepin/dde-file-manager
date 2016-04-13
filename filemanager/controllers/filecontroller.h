#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include "abstractfilecontroller.h"

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;
class AbstractFileInfo;
class FileMonitor;

class FileController : public AbstractFileController
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);

    AbstractFileInfo *createFileInfo(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfo*> getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFiles(const QList<QString> &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool renameFile(const QString &oldUrl, const QString &newUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QList<QString> &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool moveToTrash(const QList<QString> &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool cutFiles(const QList<QString> &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool pasteFile(PasteType type, const QList<QString> &urlList,
                   const QString &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFolder(const QString &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFile(const QString &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newDocument(const QString &toUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool addUrlMonitor(const QString &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const QString &url, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFileLocation(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

private slots:
    void onFileCreated(const QString &filePath);
    void onFileRemove(const QString &filePath);

private:
    FileMonitor *fileMonitor;

    QString checkDuplicateName(const QString &name) const;
};

#endif // FILECONTROLLER_H
