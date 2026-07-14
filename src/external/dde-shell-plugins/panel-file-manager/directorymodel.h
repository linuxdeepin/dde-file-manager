// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QFutureWatcher>

namespace dock {

class DirectoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int folderCount READ folderCount NOTIFY countChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY countChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationChanged)

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
        QVector<QFileInfo> pendingThumbnails;
        int generation = 0;
    };

    void loadDirectory();
    void onLoadFinished();
    void enqueueThumbnails(const QVector<QFileInfo> &files);
    void onThumbnailGenerated(const QString &sourceFilePath, const QString &thumbnailPath);
    static QString cachedThumbnailUrl(const QFileInfo &fileInfo, bool *needsGeneration);
    static LoadResult collectEntries(const QString &path, int generation);

    QString m_path;
    QVector<Entry> m_entries;
    int m_folderCount = 0;
    QStringList m_history;
    int m_historyIndex = -1;
    QFutureWatcher<LoadResult> *m_watcher = nullptr;
    int m_loadGeneration = 0;
};

}
