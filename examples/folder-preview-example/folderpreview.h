// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERPREVIEW_H
#define FOLDERPREVIEW_H

#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QPointer>
#include <QUrl>

namespace example_folderprev {

class FolderContentWidget;

/**
 * @brief Preview implementation for directories.
 *
 * Displays an at-a-glance summary of a folder's immediate contents:
 * file/sub-folder counts, total size, and a sortable file list with
 * name, type, size and last-modified date columns.
 */
class FolderPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit FolderPreview(QObject *parent = nullptr);
    ~FolderPreview() override;

    // ── AbstractBasePreview interface ─────────────────────────────────
    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;
    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

private:
    QUrl m_currentUrl;
    QString m_title;
    QPointer<FolderContentWidget> m_widget;
};

}   // namespace example_folderprev
#endif   // FOLDERPREVIEW_H
