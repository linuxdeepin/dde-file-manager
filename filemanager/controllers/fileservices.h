#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>

#include "abstractfilecontroller.h"
#include "durl.h"

typedef QPair<QString,QString> HandlerType;

class AbstractFileInfo;
class FMEvent;

class FileServices : public QObject
{
    Q_OBJECT

public:
    static FileServices *instance();

    static void setFileUrlHandler(const QString &scheme, const QString &host,
                                  AbstractFileController *controller);
    static void unsetFileUrlHandler(AbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    bool openFile(const DUrl &fileUrl) const;
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

public slots:
    void getChildren(const FMEvent &event, QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot) const;
    void openNewWindow(const DUrl &fileUrl) const;
    void openUrl(const FMEvent &event) const;

signals:
    void childrenUpdated(const FMEvent &event, const QList<AbstractFileInfo*> &list) const;
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
    void fileOpened(const DUrl &fileUrl) const;

private:
    explicit FileServices(QObject *parent = 0);
    static QList<AbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);

    static QMultiHash<HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<AbstractFileController*, HandlerType> m_handlerHash;
};

#endif // FILESERVICES_H
