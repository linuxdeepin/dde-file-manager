#ifndef FILESERVICES_H
#define FILESERVICES_H

#include "dabstractfilecontroller.h"
#include "durl.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>

#include <functional>

typedef QPair<QString,QString> HandlerType;

class DAbstractFileInfo;
class DFMEvent;
class JobController;

class DFileService : public QObject
{
    Q_OBJECT

public:
    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        m_controllerCreatorHash.insertMulti(HandlerType(scheme, host), [] {
            return (DAbstractFileController*)new T(instance());
        });
    }

    static DFileService *instance();

    static void setFileUrlHandler(const QString &scheme, const QString &host,
                                  DAbstractFileController *controller);
    static void unsetFileUrlHandler(DAbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

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
    static QList<DAbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);
    static QString getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir = QDir());

    static QMultiHash<const HandlerType, DAbstractFileController*> m_controllerHash;
    static QHash<const DAbstractFileController*, HandlerType> m_handlerHash;
    static QMultiHash<const HandlerType, std::function<DAbstractFileController*()>> m_controllerCreatorHash;
};

#endif // FILESERVICES_H
