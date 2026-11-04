// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <dock/foldercontent.h>
#include "directorymodel.h"

class QQuickItem;

namespace dfm {

class FolderPresentation : public dock::FolderPresentation
{
    Q_OBJECT
    Q_PROPERTY(dock::DirectoryModel *directoryModel READ directoryModel CONSTANT)
    Q_PROPERTY(QString directoryPath READ directoryPath NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationChanged)
    Q_PROPERTY(int iconViewMode MEMBER m_iconViewMode NOTIFY iconViewModeChanged)
public:
    explicit FolderPresentation(const QString &url, QObject *parent = nullptr);
    QString title() const override;
    QVariantList previewIcons() const override;
    QUrl iconUrl() const override;
    QUrl contentUrl() const override;
    dock::DirectoryModel *directoryModel() { return &m_browser; }
    QString directoryPath() const { return m_browser.path(); }
    bool canGoBack() const { return m_browser.canGoBack(); }
    bool canGoForward() const { return m_browser.canGoForward(); }
    Q_INVOKABLE void navigateTo(const QString &path);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();
    Q_INVOKABLE void refreshDirectory();
    Q_INVOKABLE void openFile(const QString &path);
    // True when @p path is an entry actually shown by this view.
    Q_INVOKABLE bool containsPath(const QString &path) const;
    Q_INVOKABLE void openInFileManager();
    //! Starts a system copy-drag for one entry so it can be dropped on the
    //! desktop, another file manager window, or any other drop target.
    Q_INVOKABLE void startFileDrag(const QString &path, QQuickItem *item);
signals:
    void navigationChanged();
    void iconViewModeChanged();
private:
    void adoptPath(const QString &oldPath, const QString &newPath);

    QString m_pinnedPath;
    dock::DirectoryModel m_preview;
    dock::DirectoryModel m_browser;
    int m_iconViewMode = 0;
};

class FolderProvider : public QObject, public dock::FolderContentProvider
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DS_FOLDER_CONTENT_PROVIDER_IID)
    Q_INTERFACES(dock::FolderContentProvider)
public:
    FolderProvider();
    QString normalizeId(const QString &id) const override;
    dock::FolderPresentation *create(const QString &id, QObject *parent) override;
};

}
