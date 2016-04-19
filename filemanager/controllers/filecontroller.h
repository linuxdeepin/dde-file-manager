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

    AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfo*> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool compressFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool decompressFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool moveToTrash(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool cutFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool pasteFile(PasteType type, const DUrlList &urlList,
                   const FMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFolder(const DUrl &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFile(const DUrl &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newDocument(const DUrl &toUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool addUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

private slots:
    void onFileCreated(const QString &filePath);
    void onFileRemove(const QString &filePath);

private:
    FileMonitor *fileMonitor;

    QString checkDuplicateName(const QString &name) const;
};

#endif // FILECONTROLLER_H
