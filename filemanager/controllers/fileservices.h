#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>

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

public slots:
    void getChildren(const FMEvent &event, QDir::Filters filters = QDir::AllEntries | QDir::NoDotDot) const;

signals:
    void childrenUpdated(const FMEvent &event, const QList<AbstractFileInfo*> &list) const;
    void childrenAdded(const QString &fileUrl) const;
    void childrenRemoved(const QString &fileUrl) const;
    void fileOpened(const QString &fileUrl) const;

private:
    explicit FileServices();
    static QList<AbstractFileController*> getHandlerTypeByUrl(const QString &fileUrl,
                                                              bool ignoreHost = false,
                                                              bool ignoreScheme = false);

    static QMultiHash<HandlerType, AbstractFileController*> m_controllerHash;
    static QHash<AbstractFileController*, HandlerType> m_handlerHash;

    QThread *m_thread;
};

#endif // FILESERVICES_H
