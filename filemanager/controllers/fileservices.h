#ifndef FILESERVICES_H
#define FILESERVICES_H

#include "abstractfilecontroller.h"
#include "durl.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include <QDebug>

#include <functional>

typedef QPair<QString,QString> HandlerType;

class AbstractFileInfo;
class FMEvent;
class JobController;

class FileServices : public QObject
{
    Q_OBJECT

public:
    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        m_controllerCreatorHash.insertMulti(HandlerType(scheme, host), [] {
            return (AbstractFileController*)new T(instance());
        });
    }

    static FileServices *instance();

    static void setFileUrlHandler(const QString &scheme, const QString &host,
                                  AbstractFileController *controller);
    static void unsetFileUrlHandler(AbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    bool openFile(const DUrl &fileUrl) const;
    bool compressFiles(const DUrlList &urlList) const ;
    bool decompressFile(const DUrlList urllist) const;
    bool decompressFileHere(const DUrlList urllist) const;
    bool copyFiles(const DUrlList &urlList) const;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, const FMEvent &event) const;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl) const;
    void deleteFiles(const DUrlList &urlList, const FMEvent &event) const;
    bool deleteFilesSync(const DUrlList &urlList, const FMEvent &event) const;
    void moveToTrash(const DUrlList &urlList) const;
    bool moveToTrashSync(const DUrlList &urlList) const;
    bool cutFiles(const DUrlList &urlList) const;
    void pasteFile(const FMEvent &event) const;
    void pasteFile(AbstractFileController::PasteType type,
                   const DUrlList &urlList,
                   const FMEvent &event) const;
    void restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const FMEvent &event) const;
    bool newFolder(const FMEvent &event) const;
    bool newFile(const DUrl &toUrl) const;
    bool newDocument(const DUrl &toUrl) const;

    bool addUrlMonitor(const DUrl &url) const;
    bool removeUrlMonitor(const DUrl &url) const;

    bool openFileLocation(const DUrl &fileUrl) const;

    bool createSymlink(const DUrl &fileUrl, const FMEvent &event) const;
    bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl) const;
    bool sendToDesktop(const FMEvent &event) const;

    bool openInTerminal(const DUrl &fileUrl) const;

    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl) const;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, QDir::Filters filters,
                                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) const;

    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filters, bool *ok = Q_NULLPTR);

    JobController *getChildrenJob(const DUrl &fileUrl, QDir::Filters filters) const;

public slots:
    void openNewWindow(const DUrl &fileUrl) const;
    void openUrl(const FMEvent &event) const;

signals:
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
    void childrenUpdated(const DUrl &fileUrl) const;
    void fileOpened(const DUrl &fileUrl) const;

private:
    explicit FileServices(QObject *parent = 0);
    static QList<AbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);
    static QString getSymlinkFileName(const DUrl &fileUrl);

    static QMultiHash<const HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<const AbstractFileController*, HandlerType> m_handlerHash;
    static QMultiHash<const HandlerType, std::function<AbstractFileController*()>> m_controllerCreatorHash;
};

#endif // FILESERVICES_H
