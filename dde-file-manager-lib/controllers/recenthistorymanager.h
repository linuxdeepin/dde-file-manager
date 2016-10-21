#ifndef RECENTHISTORYMANAGER_H
#define RECENTHISTORYMANAGER_H

#include <QList>
#include <QDir>
#include <QMap>

#include "basemanager.h"
#include "dabstractfilecontroller.h"

class DAbstractFileInfo;

class RecentHistoryManager : public DAbstractFileController, public BaseManager
{
    Q_OBJECT
public:
    explicit RecentHistoryManager(QObject *parent = 0);
    ~RecentHistoryManager();

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

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
