#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include "../views/filemenumanager.h"

typedef QPair<QString,QString> HandlerType;

class AbstractFileController;
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

    bool openFile(const QString &fileUrl) const;
    bool renameFile(const QString &oldUrl, const QString &newUrl) const;
    bool addUrlMonitor(const QString &url) const;
    bool removeUrlMonitor(const QString &url) const;

    AbstractFileInfo *createFileInfo(const QString &fileUrl) const;

    QString checkDuplicateName(const QString &name);

public slots:
    void getChildren(const FMEvent &event, QDir::Filters filters = QDir::AllEntries | QDir::NoDotDot) const;

    void doOpen(const QString &url);
    void doOpenNewWindow(const QString& url);
    void doOpenFileLocation(const QString &url);
    void doRename(const QString &url, int windowId);
    void doDelete(const QList<QString> & urls);
    void doCompleteDeletion(const QList<QString> & urls);
    void doSorting(FileMenuManager::MenuAction type);
    void doCopy(const QList<QString> & urls);
    void doPaste(const QString &url);
    void doCut(const QList<QString> & urls);
    void doNewFolder(const QString &url);
    void doNewDocument(const QString &url);
    void doNewFile(const QString &url);
    void doSelectAll(int windowId);
    void doRemove(const QString &url);

signals:
    void childrenUpdated(const FMEvent &event, const QList<AbstractFileInfo*> &list) const;
    void childrenAdded(const QString &fileUrl) const;
    void childrenRemoved(const QString &fileUrl) const;
    void fileOpened(const QString &fileUrl) const;


    void startMoveToTrash(const QList<QUrl> &files);
    void startCompleteDeletion(const QList<QUrl> &files);
    void startCopy(const QList<QUrl> &files, const QString &dst);

private:
    explicit FileServices(QObject *parent = 0);
    static QList<AbstractFileController*> getHandlerTypeByUrl(const QString &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);

    static QMultiHash<HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<AbstractFileController*, HandlerType> m_handlerHash;

    QThread *m_thread;
};

#endif // FILESERVICES_H
