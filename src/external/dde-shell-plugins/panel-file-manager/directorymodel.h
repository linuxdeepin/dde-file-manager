// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

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
        IconUrlRole,
        IconNameRole,
        IsDirRole,
        FileTypeRole,
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

private:
    void loadDirectory();
    static QString iconToDataUrl(const QString &iconName, int size);

    struct Entry {
        QString name;
        QString path;
        QString iconName;
        QString iconUrl;
        bool isDir;
        FileType fileType = GenericFile;
    };

    QString m_path;
    QVector<Entry> m_entries;
    int m_folderCount = 0;
    QStringList m_history;
    int m_historyIndex = -1;
};

}
