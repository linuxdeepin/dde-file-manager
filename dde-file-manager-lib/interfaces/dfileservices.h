#ifndef FILESERVICES_H
#define FILESERVICES_H

#include "dabstractfilecontroller.h"
#include "durl.h"
#include "dfmabstracteventhandler.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include <QDebug>

#include <functional>

DFM_USE_NAMESPACE

typedef QPair<QString,QString> HandlerType;
typedef QPair<QString, std::function<DAbstractFileController*()>> HandlerCreatorType;

class DAbstractFileInfo;
class JobController;
class DFMCreateGetChildrensJob;
class DFileServicePrivate;
class DFileService : public QObject, public DFMAbstractEventHandler
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
        PasteFileByClipboard = 0x00040000,
        CreateFileWatcher = 0x00080000
    };

    Q_ENUMS(FileOperatorType)
    Q_DECLARE_FLAGS(FileOperatorTypes, FileOperatorType)

    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        if (isRegisted<T>(scheme, host))
            return;

        insertToCreatorHash(HandlerType(scheme, host), HandlerCreatorType(typeid(T).name(), [=] {
            return (DAbstractFileController*)new T(instance());
        }));
    }
    static bool isRegisted(const QString &scheme, const QString &host, const std::type_info &info);
    template <class T>
    static bool isRegisted(const QString &scheme, const QString &host)
    {
        return isRegisted(scheme, host, typeid(T));
    }

    static void initHandlersByCreators();

    static DFileService *instance();

    static bool setFileUrlHandler(const QString &scheme, const QString &host,
                                  DAbstractFileController *controller);
    static void unsetFileUrlHandler(DAbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    static QList<DAbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                               bool ignoreHost = false,
                                                               bool ignoreScheme = false);

    void setFileOperatorWhitelist(FileOperatorTypes list);
    FileOperatorTypes fileOperatorWhitelist() const;
    void setFileOperatorBlacklist(FileOperatorTypes list);
    FileOperatorTypes fileOperatorBlacklist() const;

    bool openFile(const DUrl &url, const QObject *sender = 0) const;
    bool openFileByApp(const QString &appName, const DUrl &url, const QObject *sender = 0) const;
    bool compressFiles(const DUrlList &list, const QObject *sender = 0) const;
    bool decompressFile(const DUrlList &list, const QObject *sender = 0) const;
    bool decompressFileHere(const DUrlList &list, const QObject *sender = 0) const;
    bool writeFilesToClipboard(DFMGlobal::ClipboardAction action, const DUrlList &list, const QObject *sender = 0) const;
    bool renameFile(const DUrl &from, const DUrl &to, const QObject *sender = 0) const;
    bool deleteFiles(const DUrlList &list, const QObject *sender = 0) const;
    bool deleteFilesSync(const DUrlList &list, const QObject *sender = 0) const;
    void moveToTrash(const DUrlList &list, const QObject *sender = 0) const;
    DUrlList moveToTrashSync(const DUrlList &list, const QObject *sender = 0) const;
    void pasteFileByClipboard(const DUrl &targetUrl, const QObject *sender = 0) const;
    void pasteFile(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                   const DUrlList &list, const QObject *sender = 0) const;
    void restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event) const;
    bool newFolder(const DUrl &targetUrl, const QObject *sender = 0) const;
    bool newFile(const DUrl &targetUrl, const QString &fileSuffix, const QObject *sender = 0) const;
    bool openFileLocation(const DUrl &url, const QObject *sender = 0) const;

    bool createSymlink(const DUrl &fileUrl, const QObject *sender = 0) const;
    bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, const QObject *sender = 0) const;
    bool sendToDesktop(const DUrlList &urlList, const QObject *sender = 0) const;

    bool shareFolder(const DUrl &fileUrl, const QString &name, bool isWritable = false, bool allowGuest = false, const QObject *sender = 0);
    bool unShareFolder(const DUrl &fileUrl, const QObject *sender = 0) const;
    bool openInTerminal(const DUrl &fileUrl, const QObject *sender = 0) const;

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, const QObject *sender = 0) const;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, const QObject *sender = 0) const;

    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, const QObject *sender = 0);

    JobController *getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, const QObject *sender = 0) const;

    DAbstractFileWatcher *createFileWatcher(const DUrl &fileUrl, QObject *parent = 0, const QObject *sender = 0) const;

    bool isAvfsMounted() const;

public slots:
    void openNewWindow(const DFMEvent& event, const bool &isNewWindow = true) const;
    void openInCurrentWindow(const DFMEvent& event) const;
    void openUrl(const DFMEvent &event, const bool& isOpenInNewWindow = true,const bool& isOpenInCurrentWindow = false) const;

signals:
    void fileOpened(const DUrl &fileUrl) const;

private slots:
    void laterRequestSelectFiles(const DFMEvent &event) const;

private:
    explicit DFileService(QObject *parent = 0);
    ~DFileService();

    bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = 0) Q_DECL_OVERRIDE;

    static QString getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir = QDir());
    static void insertToCreatorHash(const HandlerType &type, const HandlerCreatorType &creator);

    QScopedPointer<DFileServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFileService)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFileService::FileOperatorTypes)

#endif // FILESERVICES_H
