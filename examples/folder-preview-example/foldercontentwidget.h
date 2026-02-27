// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERCONTENTWIDGET_H
#define FOLDERCONTENTWIDGET_H

#include <QFrame>
#include <QUrl>
#include <QFileInfo>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
class QLabel;
class QProcess;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace example_folderprev {

// ── Internal data structures ──────────────────────────────────────────

/**
 * @brief Holds the scanned results of a directory's immediate contents.
 *
 * totalSize is intentionally NOT populated here; it is filled separately
 * by running `du -sb` so that sub-directory sizes are included.
 */
struct FolderScanResult
{
    QList<QFileInfo> entries;   ///< All immediate children (files + dirs)
    int fileCount { 0 };        ///< Number of regular files
    int dirCount { 0 };         ///< Number of sub-directories
};

// ── StatBadge ─────────────────────────────────────────────────────────

/**
 * @brief A small rounded badge showing a numeric value and a label,
 *        used in the header info-strip (e.g. "5 Files").
 */
class StatBadge : public QFrame
{
    Q_OBJECT
public:
    explicit StatBadge(const QString &iconText,
                       const QString &label,
                       QWidget *parent = nullptr);

    /** @brief Update the displayed value. */
    void setValue(const QString &value);

private:
    QLabel *m_iconLabel { nullptr };
    QLabel *m_valueLabel { nullptr };
    QLabel *m_captionLabel { nullptr };
};

// ── FolderContentWidget ───────────────────────────────────────────────

/**
 * @brief Main widget composing the folder preview.
 *
 * Layout (top → bottom):
 *   ┌───────────────────────────────────┐
 *   │  Header: icon · name · path ·     │
 *   │          stat badges              │
 *   ├───────────────────────────────────┤
 *   │  File list tree (scrollable)      │
 *   └───────────────────────────────────┘
 *
 * Directory scanning is performed asynchronously via QtConcurrent
 * to keep the UI responsive on large folders.
 */
class FolderContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FolderContentWidget(QWidget *parent = nullptr);
    ~FolderContentWidget() override;

    /**
     * @brief Load and display the contents of @p url.
     * @param url  Must be a local directory URL (file:///).
     */
    void loadFolder(const QUrl &url);

    // ── Size hints ────────────────────────────────────────────────────
    /** @brief Returns the preferred size so FilePreviewDialog sizes the window correctly. */
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onScanFinished();
    /** @brief Called when the `du -sb` process finishes; parses output and updates the size badge. */
    void onDuFinished(int exitCode, int exitStatus);

private:
    // ── UI helpers ────────────────────────────────────────────────────
    void setupUi();
    void applyStyleSheet();

    // ── Data population ───────────────────────────────────────────────
    void populateList(const FolderScanResult &result);
    void updateStats(const FolderScanResult &result);
    QTreeWidgetItem *makeItem(const QFileInfo &fi);

    // ── Utilities ─────────────────────────────────────────────────────
    static QString formatSize(qint64 bytes);
    static QString formatDate(const QDateTime &dt);
    static QString mimeDescription(const QFileInfo &fi);

    // ── Widgets ───────────────────────────────────────────────────────
    QLabel *m_folderIconLabel { nullptr };
    QLabel *m_nameLabel { nullptr };
    QLabel *m_pathLabel { nullptr };

    StatBadge *m_fileBadge { nullptr };
    StatBadge *m_dirBadge { nullptr };
    StatBadge *m_sizeBadge { nullptr };

    QFrame *m_separator { nullptr };
    QTreeWidget *m_listWidget { nullptr };
    QLabel *m_loadingLabel { nullptr };

    // ── Async scan (file list) ────────────────────────────────────────
    QFutureWatcher<FolderScanResult> *m_watcher { nullptr };

    // ── Async scan (total size via `du -sb`) ──────────────────────────
    QProcess *m_duProcess { nullptr };

    QUrl m_currentUrl;
};

}   // namespace example_folderprev
#endif   // FOLDERCONTENTWIDGET_H
