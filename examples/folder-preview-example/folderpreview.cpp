// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderpreview.h"
#include "foldercontentwidget.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>

using namespace example_folderprev;
DFMBASE_USE_NAMESPACE

FolderPreview::FolderPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    qDebug() << "FolderPreview: instance created";
}

FolderPreview::~FolderPreview()
{
    qDebug() << "FolderPreview: instance destroyed";
    if (m_widget)
        m_widget->deleteLater();
}

bool FolderPreview::setFileUrl(const QUrl &url)
{
    if (!url.isValid() || !url.isLocalFile()) {
        qWarning() << "FolderPreview: invalid or non-local URL:" << url;
        return false;
    }

    const QString localPath = url.toLocalFile();
    if (!QFileInfo(localPath).isDir()) {
        qWarning() << "FolderPreview: path is not a directory:" << localPath;
        return false;
    }

    if (m_currentUrl == url) {
        qDebug() << "FolderPreview: URL unchanged, skipping reload";
        return true;
    }

    m_currentUrl = url;
    m_title = QFileInfo(localPath).fileName();
    if (m_title.isEmpty())
        m_title = localPath;   // root "/"

    qDebug() << "FolderPreview: loading folder:" << localPath;

    // Widget is created lazily in contentWidget(); just reload data here.
    if (m_widget)
        m_widget->loadFolder(url);

    Q_EMIT titleChanged();
    return true;
}

QUrl FolderPreview::fileUrl() const
{
    return m_currentUrl;
}

QWidget *FolderPreview::contentWidget() const
{
    if (!m_widget) {
        // Lazy-create the widget on first call.
        auto *self = const_cast<FolderPreview *>(this);
        self->m_widget = new FolderContentWidget;

        if (m_currentUrl.isValid())
            self->m_widget->loadFolder(m_currentUrl);
    }
    return m_widget;
}

QString FolderPreview::title() const
{
    return m_title;
}

bool FolderPreview::showStatusBarSeparator() const
{
    return true;
}
