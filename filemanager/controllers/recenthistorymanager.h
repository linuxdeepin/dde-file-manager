#ifndef RECENTHISTORYMANAGER_H
#define RECENTHISTORYMANAGER_H

#include <QList>
#include <QDir>
#include <QMap>

#include "basemanager.h"
#include "abstractfilecontroller.h"

class AbstractFileInfo;

class RecentHistoryManager : public AbstractFileController, public BaseManager
{
    Q_OBJECT
public:
    explicit RecentHistoryManager(QObject *parent = 0);
    ~RecentHistoryManager();

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;
    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    static QString cachePath();

private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);

    void removeRecentFiles(const DUrlList &urlList);
    void clearRecentFiles();

private slots:
    void addOpenedFile(const DUrl &url);

private:
    QList<DUrl> m_openedFileList;
    QMap<DUrl, QDateTime> m_lastFileOpenedTime;
};

#endif // RECENTHISTORYMANAGER_H
