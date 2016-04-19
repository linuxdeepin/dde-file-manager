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
    bool decompressFile(const DUrl &fileUrl) const;
    bool copyFiles(const DUrlList &urlList) const;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl) const;
    void deleteFiles(const DUrlList &urlList) const;
    void moveToTrash(const DUrlList &urlList) const;
    bool cutFiles(const DUrlList &urlList) const;
    void pasteFile(const FMEvent &event) const;
    void pasteFile(AbstractFileController::PasteType type,
                   const DUrlList &urlList,
                   const FMEvent &event) const;
    bool newFolder(const DUrl &toUrl) const;
    bool newFile(const DUrl &toUrl) const;
    bool newDocument(const DUrl &toUrl) const;

    bool addUrlMonitor(const DUrl &url) const;
    bool removeUrlMonitor(const DUrl &url) const;

    bool openFileLocation(const DUrl &fileUrl) const;

    AbstractFileInfo *createFileInfo(const DUrl &fileUrl) const;

    const QList<AbstractFileInfo*> getChildren(const DUrl &fileUrl, QDir::Filters filters, bool *ok = 0) const;

public slots:
    void getChildren(const FMEvent &event, QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot) const;
    void openNewWindow(const DUrl &fileUrl) const;
    void openUrl(const FMEvent &event) const;

signals:
    void updateChildren(const FMEvent &event, const QList<AbstractFileInfo*> &list) const;
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
    void childrenUpdated(const DUrl &fileUrl) const;
    void fileOpened(const DUrl &fileUrl) const;

private:
    explicit FileServices(QObject *parent = 0);
    static QList<AbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);

    static QMultiHash<const HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<const AbstractFileController*, HandlerType> m_handlerHash;
    static QMultiHash<const HandlerType, std::function<AbstractFileController*()>> m_controllerCreatorHash;
};

#endif // FILESERVICES_H
