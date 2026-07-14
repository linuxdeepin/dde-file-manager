// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <dde-shell/applet.h>
#include <dde-shell/dsglobal.h>

#include "directorymodel.h"

namespace dock {

class FileManagerPlugin : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(dock::DirectoryModel *directoryModel READ directoryModel CONSTANT)
    Q_PROPERTY(int folderCount READ folderCount NOTIFY folderCountChanged)
    Q_PROPERTY(int displayFolderCount READ displayFolderCount NOTIFY folderCountChanged)
    Q_PROPERTY(QString directoryPath READ directoryPath NOTIFY directoryPathChanged)
    Q_PROPERTY(int iconViewMode READ iconViewMode WRITE setIconViewMode NOTIFY iconViewModeChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationChanged)
    Q_PROPERTY(QStringList previewIconNames READ previewIconNames NOTIFY previewIconNamesChanged)

public:
    explicit FileManagerPlugin(QObject *parent = nullptr);
    virtual bool init() override;

    dock::DirectoryModel *directoryModel() const;
    int folderCount() const;
    int displayFolderCount() const;
    QString directoryPath() const;

    int iconViewMode() const;
    void setIconViewMode(int mode);

    Q_INVOKABLE void openFile(const QString &filePath);
    Q_INVOKABLE void refreshDirectory();
    Q_INVOKABLE void navigateTo(const QString &path);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();
    Q_INVOKABLE bool isDirectory(const QString &path) const;
    Q_INVOKABLE bool isFile(const QString &path) const;

    QStringList previewIconNames() const;
    bool canGoBack() const;
    bool canGoForward() const;

Q_SIGNALS:
    void folderCountChanged();
    void directoryPathChanged();
    void iconViewModeChanged();
    void navigationChanged();
    void previewIconNamesChanged();
    void thumbnailChanged(int row);

private:
    int m_iconViewMode = 0;
    DirectoryModel *m_directoryModel = nullptr;
};

}
