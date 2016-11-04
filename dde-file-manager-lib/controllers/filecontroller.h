#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include "dabstractfilecontroller.h"

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;
class DAbstractFileInfo;
class FileMonitor;

class FileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);

    static bool findExecutable(const QString & executableName, const QStringList & paths = QStringList());

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileByApp(const DUrl &fileUrl, const QString& app, bool &accepted) const Q_DECL_OVERRIDE;
    bool compressFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool decompressFile(const DUrlList &fileUrlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool decompressFileHere(const DUrlList &fileUrlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(PasteType type, const DUrl &targetUrl, const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFolder(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool newFile(const DUrl &toUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool newDocument(const DUrl &toUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool addUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openInTerminal(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const Q_DECL_OVERRIDE;

private slots:
    void onFileCreated(const QString &filePath);
    void onFileRemove(const QString &filePath);
    void onFileInfoChanged(const QString &filePath);

private:
    FileMonitor *fileMonitor;

    QString checkDuplicateName(const QString &name) const;
};

#endif // FILECONTROLLER_H
