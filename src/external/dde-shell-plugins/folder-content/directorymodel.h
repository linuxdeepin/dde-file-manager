// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QFileSystemWatcher>
#include <QTimer>

namespace dock {

class DirectoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int folderCount READ folderCount NOTIFY countChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY countChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationChanged)
    // True while the browsed directory sits on a network mount whose host is
    // unreachable: the model stays empty and file-system access is skipped
    // (see NetworkGuard) so the popup cannot hang the dock UI.
    Q_PROPERTY(bool networkBusy READ networkBusy NOTIFY networkBusyChanged)

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        PathRole = Qt::UserRole + 1,
        IconNameRole,
        IsDirRole,
        FileTypeRole,
        ThumbnailUrlRole,
    };

    enum FileType {
        GenericFile = 0,
        Folder,
        ImageFile,
        VideoFile,
        AudioFile,
        ScriptFile,
        DesktopFile,
    };
    Q_ENUM(FileType)

    explicit DirectoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString path() const;
    void setPath(const QString &path);

    int folderCount() const;
    int totalCount() const;
    bool networkBusy() const { return m_networkBusy; }
    bool canGoBack() const;
    bool canGoForward() const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE void navigateTo(const QString &path);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();

Q_SIGNALS:
    void pathChanged();
    void countChanged();
    void navigationChanged();
    void thumbnailChanged(int row);
    // Emitted when the watched path disappeared but a sibling with the same
    // inode took its place, i.e. the directory was renamed in place.
    void pathMoved(const QString &oldPath, const QString &newPath);
    // Emitted when the watched directory was deleted while its parent stayed
    // in place, so the pin can be dropped instead of showing an empty folder.
    void pathLost();
    void networkBusyChanged();

private:
    struct Entry {
        QString name;
        QString path;
        QString iconName;
        bool isDir;
        FileType fileType = GenericFile;
        QString thumbnailUrl;
    };

    struct LoadResult {
        QVector<Entry> entries;
        int folderCount = 0;
        int generation = 0;
    };

    void loadDirectory();
    void applyResult(LoadResult result);
    void updateWatches();
    void setNetworkBusy(bool busy);
    void enqueueThumbnails(const QVector<QFileInfo> &files);
    void onThumbnailGenerated(const QString &sourceFilePath, const QString &thumbnailPath);
    static QString cachedThumbnailUrl(const QFileInfo &fileInfo, bool *needsGeneration);
    static LoadResult collectEntries(const QString &path, int generation);
    // Look in the parent directory for an entry with the remembered inode.
    QString findRenamedPath() const;
    // A deleted entry keeps its parent directory; a missing parent means the
    // whole tree is temporarily unreachable and the pin should be kept.
    bool parentDirectoryAlive() const;

    QString m_path;
    QVector<Entry> m_entries;
    int m_folderCount = 0;
    QStringList m_history;
    int m_historyIndex = -1;
    QFileSystemWatcher m_fileWatcher;
    QTimer m_reloadTimer;
    // Short grace period so a delete/re-create cycle does not flap the pin.
    QTimer m_lossTimer;
    int m_loadGeneration = 0;
    // Identity of m_path, captured when it was set, used to follow a rename.
    quint64 m_pathDevice = 0;
    quint64 m_pathInode = 0;
    // Set while the current mount is unreachable; every file-system touch on
    // the GUI thread is skipped until the host responds again.
    bool m_networkBusy = false;
};

}
