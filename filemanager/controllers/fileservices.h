#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>

#include "abstractfilecontroller.h"

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

    bool openFile(const QString &fileUrl) const;
    bool copyFiles(const QList<QString> &urlList) const;
    bool renameFile(const QString &oldUrl, const QString &newUrl) const;
    void deleteFiles(const QList<QString> &urlList) const;
    void moveToTrash(const QList<QString> &urlList) const;
    bool cutFiles(const QList<QString> &urlList) const;
    void pasteFile(const FMEvent &event) const;
    void pasteFile(AbstractFileController::PasteType type,
                   const QList<QString> &urlList,
                   const FMEvent &event) const;
    bool newFolder(const QString &toUrl) const;
    bool newFile(const QString &toUrl) const;
    bool newDocument(const QString &toUrl) const;

    bool addUrlMonitor(const QString &url) const;
    bool removeUrlMonitor(const QString &url) const;

    bool openFileLocation(const QString &fileUrl) const;

    AbstractFileInfo *createFileInfo(const QString &fileUrl) const;

public slots:
    void getChildren(const FMEvent &event, QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot) const;
    void openNewWindow(const QString &fileUrl) const;
    void openUrl(const FMEvent &event) const;

signals:
    void childrenUpdated(const FMEvent &event, const QList<AbstractFileInfo*> &list) const;
    void childrenAdded(const QString &fileUrl) const;
    void childrenRemoved(const QString &fileUrl) const;
    void fileOpened(const QString &fileUrl) const;

private:
    explicit FileServices(QObject *parent = 0);
    static QList<AbstractFileController*> getHandlerTypeByUrl(const QString &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);

    static QMultiHash<HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<AbstractFileController*, HandlerType> m_handlerHash;
};

#endif // FILESERVICES_H
