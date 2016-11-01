#ifndef FILESERVICES_H
#define FILESERVICES_H

#include "dabstractfilecontroller.h"
#include "durl.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include <QDebug>

#include <functional>

typedef QPair<QString,QString> HandlerType;

class DAbstractFileInfo;
class DFMEvent;
class JobController;
class DFileServicePrivate;
class DFileService : public QObject
{
    Q_OBJECT

public:
    enum FileOperatorType {
        OpenFile = 0x00000001,
        CompressFiles = 0x00000002,
        DecompressFile = 0x00000004,
        DecompressFileHere = 0x00000008,
        CopyFilesToClipboard = 0x00000010,
        RenameFile = 0x00000020,
        DeleteFiles = 0x00000040,
        MoveToTrash = 0x00000080,
        CutFilesToClipboard = 0x00000100,
        PasteFile = 0x00000200,
        RestoreFile = 0x00000400,
        NewFolder = 0x00000800,
        NewDocument = 0x00001000,
        OpenFileLocation = 0x00002000,
        CreateSymlink = 0x00004000,
        OpenInTerminal = 0x00008000,
        OpenNewWindow = 0x00010000,
        OpenUrl = 0x00020000,
        PasteFileByClipboard = 0x00040000
    };

    Q_ENUMS(FileOperatorType)
    Q_DECLARE_FLAGS(FileOperatorTypes, FileOperatorType)

    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        m_controllerCreatorHash.insertMulti(HandlerType(scheme, host), [=] {
            return (DAbstractFileController*)new T(instance());
        });
    }

    static DFileService *instance();

    static void setFileUrlHandler(const QString &scheme, const QString &host,
                                  DAbstractFileController *controller);
    static void unsetFileUrlHandler(DAbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    void setFileOperatorWhitelist(FileOperatorTypes list);
    FileOperatorTypes fileOperatorWhitelist() const;
    void setFileOperatorBlacklist(FileOperatorTypes list);
    FileOperatorTypes fileOperatorBlacklist() const;

    bool openFile(const DUrl &fileUrl) const;
    bool compressFiles(const DUrlList &urlList) const ;
    bool decompressFile(const DUrlList urllist) const;
    bool decompressFileHere(const DUrlList urllist) const;
    bool copyFilesToClipboard(const DUrlList &urlList) const;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, const DFMEvent &event) const;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl) const;
    void deleteFiles(const DFMEvent &event) const;
    bool deleteFilesSync(const DFMEvent &event) const;
    void moveToTrash(const DFMEvent &event) const;
    DUrlList moveToTrashSync(const DFMEvent &event) const;
    bool cutFilesToClipboard(const DUrlList &urlList) const;
    void pasteFileByClipboard(const DUrl &tarUrl, const DFMEvent &event) const;
    void pasteFile(DAbstractFileController::PasteType type, const DUrl &tarUrl, const DFMEvent &event) const;
    void restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event) const;
    bool newFolder(const DFMEvent &event) const;
    bool newFile(const DUrl &toUrl) const;
    bool newDocument(const DUrl &toUrl) const;

    bool addUrlMonitor(const DUrl &url) const;
    bool removeUrlMonitor(const DUrl &url) const;

    bool openFileLocation(const DUrl &fileUrl) const;

    bool createSymlink(const DUrl &fileUrl, const DFMEvent &event) const;
    bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl) const;
    bool sendToDesktop(const DFMEvent &event) const;

    bool openInTerminal(const DUrl &fileUrl) const;

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl) const;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) const;

    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, bool *ok = Q_NULLPTR);

    JobController *getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) const;

public slots:
    void openNewWindow(const DUrl &fileUrl) const;
    void openUrl(const DFMEvent &event) const;

signals:
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
    void childrenUpdated(const DUrl &fileUrl) const;
    void fileOpened(const DUrl &fileUrl) const;

private slots:
    void laterRequestSelectFiles(const DFMEvent &event) const;

private:
    explicit DFileService(QObject *parent = 0);
    ~DFileService();
    static QList<DAbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);
    static QString getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir = QDir());

    static QMultiHash<const HandlerType, std::function<DAbstractFileController*()>> m_controllerCreatorHash;

    QScopedPointer<DFileServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFileService)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFileService::FileOperatorTypes)

#endif // FILESERVICES_H
