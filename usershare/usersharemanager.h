#ifndef USERSHAREMANAGER_H
#define USERSHAREMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>
#include "shareinfo.h"

class UserShareManager : public QObject
{
    Q_OBJECT
public:
    explicit UserShareManager(QObject *parent = 0);
    ~UserShareManager();

    inline static QString UserSharePath(){
        return "/var/lib/samba/usershares";
    }

    void initConnect();
    QString getCacehPath();

    ShareInfo getShareInfoByPath(const QString& path) const;
    ShareInfo getsShareInfoByShareName(const QString& shareName) const;
    QString getShareNameByPath(const QString& path) const;
    ShareInfoList shareInfoList() const;
    bool hasValidShareFolders() const;
    bool isShareFile(const QString &filePath) const;

    static void writeCacheToFile(const QString &path, const QString &content);
    static QString readCacheFromFile(const QString &path);

signals:
    void userShareChanged(const int& count);
    void userShareAdded(const QString& path);
    void userShareDeleted(const QString& path);

public slots:
    void handleShareChanged();
    void updateUserShareInfo();

    void addUserShare(const ShareInfo& info);

    void deleteUserShareByShareName(const QString& shareName);
    void deleteUserShare(const ShareInfo& info);
    void deleteUserShareByPath(const QString& path);

private:
    void loadUserShareInfoPathNames();
    void saveUserShareInfoPathNames();

    QFileSystemWatcher* m_fileSystemWatcher=NULL;
    QMap<QString, ShareInfo> m_shareInfos = {};
    QMap<QString, QString> m_sharePathByFilePath = {};
    QMap<QString, QStringList> m_sharePathToNames = {};
};

#endif // USERSHAREMANAGER_H
