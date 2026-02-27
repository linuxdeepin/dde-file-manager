// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldercontentwidget.h"

#include <QDir>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QProcess>

#include <QLabel>
#include <QFrame>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QPixmap>
#include <QIcon>
#include <QSizePolicy>
#include <QScrollBar>
#include <QPainter>

using namespace example_folderprev;

// ═══════════════════════════════════════════════════════════════════════
// StatBadge
// ═══════════════════════════════════════════════════════════════════════

StatBadge::StatBadge(const QString &iconText,
                     const QString &label,
                     QWidget *parent)
    : QFrame(parent)
{
    setObjectName("StatBadge");

    // ── Layout ────────────────────────────────────────────────────────
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 10, 14, 10);
    mainLayout->setSpacing(2);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Icon emoji / unicode symbol
    m_iconLabel = new QLabel(iconText, this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    QFont iconFont = m_iconLabel->font();
    iconFont.setPointSize(18);
    m_iconLabel->setFont(iconFont);

    // Numeric value
    m_valueLabel = new QLabel("—", this);
    m_valueLabel->setObjectName("BadgeValue");
    m_valueLabel->setAlignment(Qt::AlignCenter);
    QFont valFont = m_valueLabel->font();
    valFont.setPointSize(14);
    valFont.setBold(true);
    m_valueLabel->setFont(valFont);

    // Text label below value
    m_captionLabel = new QLabel(label, this);
    m_captionLabel->setObjectName("BadgeCaption");
    m_captionLabel->setAlignment(Qt::AlignCenter);
    QFont capFont = m_captionLabel->font();
    capFont.setPointSize(9);
    m_captionLabel->setFont(capFont);

    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_valueLabel);
    mainLayout->addWidget(m_captionLabel);

    setMinimumWidth(90);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void StatBadge::setValue(const QString &value)
{
    m_valueLabel->setText(value);
}

// ═══════════════════════════════════════════════════════════════════════
// FolderContentWidget – implementation
// ═══════════════════════════════════════════════════════════════════════

FolderContentWidget::FolderContentWidget(QWidget *parent)
    : QWidget(parent)
    , m_watcher(new QFutureWatcher<FolderScanResult>(this))
    , m_duProcess(new QProcess(this))
{
    setupUi();
    applyStyleSheet();

    connect(m_watcher, &QFutureWatcher<FolderScanResult>::finished,
            this, &FolderContentWidget::onScanFinished);

    // du -sb gives the real recursive disk usage in bytes (1 byte blocks)
    connect(m_duProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FolderContentWidget::onDuFinished);
}

FolderContentWidget::~FolderContentWidget()
{
    if (m_watcher->isRunning()) {
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
    if (m_duProcess->state() != QProcess::NotRunning) {
        m_duProcess->kill();
        m_duProcess->waitForFinished(500);
    }
}

// ── Size hints ───────────────────────────────────────────────────────

QSize FolderContentWidget::sizeHint() const
{
    // Provide a comfortable default so FilePreviewDialog's setFixedSize call
    // gets a meaningful size rather than whatever Qt's layout minimum computes.
    return QSize(720, 540);
}

QSize FolderContentWidget::minimumSizeHint() const
{
    return QSize(560, 400);
}

// ── Public ────────────────────────────────────────────────────────────

void FolderContentWidget::loadFolder(const QUrl &url)
{
    if (!url.isValid() || !url.isLocalFile())
        return;

    const QString path = url.toLocalFile();
    m_currentUrl = url;

    // Update header labels immediately for responsiveness
    QFileInfo folderInfo(path);
    m_nameLabel->setText(folderInfo.fileName().isEmpty() ? path : folderInfo.fileName());
    m_pathLabel->setText(path);

    // Set big folder icon
    QFileIconProvider iconProvider;
    QIcon dirIcon = iconProvider.icon(QFileIconProvider::Folder);
    m_folderIconLabel->setPixmap(dirIcon.pixmap(64, 64));

    // Reset stat badges
    m_fileBadge->setValue("…");
    m_dirBadge->setValue("…");
    m_sizeBadge->setValue("…");

    // Show loading indicator, hide list
    m_loadingLabel->setVisible(true);
    m_listWidget->setVisible(false);
    m_listWidget->clear();

    qDebug() << "FolderContentWidget: starting async scan of" << path;

    // ── Launch async file-list scan (QtConcurrent) ────────────────────
    if (m_watcher->isRunning()) {
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
    m_watcher->setFuture(QtConcurrent::run([path]() -> FolderScanResult {
        FolderScanResult result;
        QDir dir(path);
        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
        dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
        const QFileInfoList entries = dir.entryInfoList();
        for (const QFileInfo &fi : entries) {
            result.entries.append(fi);
            if (fi.isDir())
                ++result.dirCount;
            else
                ++result.fileCount;
        }
        return result;
    }));

    // ── Launch `du -sb <path>` for accurate recursive total size ──────
    // -s : summarise (single line output)  -b : bytes (apparent size)
    if (m_duProcess->state() != QProcess::NotRunning) {
        m_duProcess->kill();
        m_duProcess->waitForFinished(500);
    }
    m_duProcess->start("du", { "-sb", path });
    qDebug() << "FolderContentWidget: started du -sb for" << path;
}

// ── Private slots ─────────────────────────────────────────────────────

void FolderContentWidget::onScanFinished()
{
    if (m_watcher->isCanceled())
        return;

    const FolderScanResult result = m_watcher->result();
    qDebug() << "FolderContentWidget: scan finished,"
             << result.fileCount << "files,"
             << result.dirCount << "dirs";

    // Update file/folder counts; size badge stays as "…" until du finishes
    updateStats(result);
    populateList(result);

    m_loadingLabel->setVisible(false);
    m_listWidget->setVisible(true);
}

void FolderContentWidget::onDuFinished(int exitCode, int /*exitStatus*/)
{
    if (exitCode != 0) {
        qWarning() << "FolderContentWidget: du exited with code" << exitCode;
        m_sizeBadge->setValue(tr("N/A"));
        return;
    }

    // Output format: "<bytes>\t<path>\n"
    const QByteArray raw = m_duProcess->readAllStandardOutput();
    const QByteArrayList parts = raw.trimmed().split('\t');
    if (parts.isEmpty()) {
        m_sizeBadge->setValue(tr("N/A"));
        return;
    }

    bool ok = false;
    const qint64 bytes = parts.first().toLongLong(&ok);
    if (!ok || bytes < 0) {
        m_sizeBadge->setValue(tr("N/A"));
        return;
    }

    qDebug() << "FolderContentWidget: du reported total size" << bytes << "bytes";
    m_sizeBadge->setValue(formatSize(bytes));
}

// ── Private helpers ───────────────────────────────────────────────────

void FolderContentWidget::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Header panel ──────────────────────────────────────────────────
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("HeaderFrame");
    headerFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(16);

    // Left: folder icon
    m_folderIconLabel = new QLabel(headerFrame);
    m_folderIconLabel->setFixedSize(64, 64);
    m_folderIconLabel->setAlignment(Qt::AlignCenter);
    m_folderIconLabel->setObjectName("FolderIcon");

    // Right: name + path + badges
    auto *infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(4);

    m_nameLabel = new QLabel(headerFrame);
    m_nameLabel->setObjectName("FolderName");
    m_nameLabel->setWordWrap(false);
    QFont nameFont = m_nameLabel->font();
    nameFont.setPointSize(15);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);

    m_pathLabel = new QLabel(headerFrame);
    m_pathLabel->setObjectName("FolderPath");
    m_pathLabel->setWordWrap(false);
    QFont pathFont = m_pathLabel->font();
    pathFont.setPointSize(10);
    m_pathLabel->setFont(pathFont);

    // Stat badges row
    auto *badgeRow = new QHBoxLayout;
    badgeRow->setSpacing(8);
    badgeRow->setContentsMargins(0, 6, 0, 0);

    m_fileBadge = new StatBadge("📄", tr("Files"), headerFrame);
    m_dirBadge = new StatBadge("📁", tr("Folders"), headerFrame);
    m_sizeBadge = new StatBadge("💾", tr("Total Size"), headerFrame);

    badgeRow->addWidget(m_fileBadge);
    badgeRow->addWidget(m_dirBadge);
    badgeRow->addWidget(m_sizeBadge);
    badgeRow->addStretch();

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_pathLabel);
    infoLayout->addLayout(badgeRow);

    headerLayout->addWidget(m_folderIconLabel, 0, Qt::AlignTop);
    headerLayout->addLayout(infoLayout, 1);

    // ── Separator ─────────────────────────────────────────────────────
    m_separator = new QFrame(this);
    m_separator->setObjectName("Separator");
    m_separator->setFrameShape(QFrame::HLine);
    m_separator->setFixedHeight(1);

    // ── Loading label ─────────────────────────────────────────────────
    m_loadingLabel = new QLabel(tr("Loading…"), this);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setObjectName("LoadingLabel");
    m_loadingLabel->setVisible(false);

    // ── File list tree ────────────────────────────────────────────────
    m_listWidget = new QTreeWidget(this);
    m_listWidget->setObjectName("FileList");
    m_listWidget->setColumnCount(4);
    m_listWidget->setHeaderLabels({ tr("Name"), tr("Type"), tr("Size"), tr("Modified") });
    m_listWidget->setRootIsDecorated(false);
    m_listWidget->setUniformRowHeights(true);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setIconSize(QSize(20, 20));
    m_listWidget->setSortingEnabled(true);
    m_listWidget->sortByColumn(0, Qt::AscendingOrder);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setMinimumHeight(260);   // guarantee rows are visible
    m_listWidget->setVisible(false);

    // Column widths
    QHeaderView *header = m_listWidget->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(60);

    // ── Assemble root layout ──────────────────────────────────────────
    rootLayout->addWidget(headerFrame);
    rootLayout->addWidget(m_separator);
    rootLayout->addWidget(m_loadingLabel, 1);
    rootLayout->addWidget(m_listWidget, 1);
}

void FolderContentWidget::applyStyleSheet()
{
    // Detect dark / light palette to pick appropriate colours.
    const QPalette pal = QApplication::palette();
    const bool isDark  = pal.color(QPalette::Window).lightness() < 128;

    // Colours
    const QString headerBg  = isDark ? "#2A2E3A" : "#F0F4FF";
    const QString badgeBg   = isDark ? "#363B4D" : "#FFFFFF";
    const QString badgeBdr  = isDark ? "#4A5170" : "#D8E0F0";
    const QString nameColor = isDark ? "#E8EEFF" : "#1A2348";
    const QString pathColor = isDark ? "#8E9BBE" : "#606880";
    const QString capColor  = isDark ? "#7B89B2" : "#7080A0";
    const QString sepColor  = isDark ? "#3A4060" : "#DCE4F4";
    const QString altRow    = isDark ? "#252933" : "#F6F8FC";
    const QString selRow    = isDark ? "#364068" : "#D6E2FF";
    const QString headerHdr = isDark ? "#2A2E3A" : "#EEF2FC";

    setStyleSheet(QString(R"(
        FolderContentWidget {
            background: transparent;
        }

        /* ── Header ─────────────────────────────────────────── */
        QFrame#HeaderFrame {
            background-color: %1;
            border: none;
        }

        QLabel#FolderName {
            color: %4;
        }

        QLabel#FolderPath {
            color: %5;
        }

        /* ── Stat badges ─────────────────────────────────────── */
        QFrame#StatBadge {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 8px;
        }

        QLabel#BadgeValue {
            color: %4;
        }

        QLabel#BadgeCaption {
            color: %6;
        }

        /* ── Separator ───────────────────────────────────────── */
        QFrame#Separator {
            background-color: %7;
            border: none;
        }

        /* ── File list ───────────────────────────────────────── */
        QTreeWidget#FileList {
            background: transparent;
            border: none;
            outline: none;
            alternate-background-color: %8;
        }

        QTreeWidget#FileList::item {
            padding: 4px 2px;
            border: none;
        }

        QTreeWidget#FileList::item:selected {
            background-color: %9;
            color: %4;
        }

        QTreeWidget#FileList::item:hover:!selected {
            background-color: %3;
        }

        QHeaderView::section {
            background-color: %10;
            color: %5;
            padding: 6px 8px;
            border: none;
            border-bottom: 1px solid %7;
            font-weight: 600;
        }

        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: %3;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }

        QLabel#LoadingLabel {
            color: %6;
            font-size: 13px;
        }
    )")
    .arg(headerBg)   // %1
    .arg(badgeBg)    // %2
    .arg(badgeBdr)   // %3
    .arg(nameColor)  // %4
    .arg(pathColor)  // %5
    .arg(capColor)   // %6
    .arg(sepColor)   // %7
    .arg(altRow)     // %8
    .arg(selRow)     // %9
    .arg(headerHdr)  // %10
    );
}

void FolderContentWidget::updateStats(const FolderScanResult &result)
{
    m_fileBadge->setValue(QString::number(result.fileCount));
    m_dirBadge->setValue(QString::number(result.dirCount));
    // Size badge is updated later by onDuFinished once `du -sb` completes
}

void FolderContentWidget::populateList(const FolderScanResult &result)
{
    m_listWidget->setUpdatesEnabled(false);
    m_listWidget->clear();

    // Sort: directories first, then files, both alphabetically
    QList<QFileInfo> sorted = result.entries;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const QFileInfo &a, const QFileInfo &b) {
                         if (a.isDir() != b.isDir())
                             return a.isDir();
                         return a.fileName().compare(b.fileName(),
                                                     Qt::CaseInsensitive) < 0;
                     });

    for (const QFileInfo &fi : sorted)
        m_listWidget->addTopLevelItem(makeItem(fi));

    m_listWidget->setUpdatesEnabled(true);
    qDebug() << "FolderContentWidget: populated list with"
             << result.entries.size() << "entries";
}

QTreeWidgetItem *FolderContentWidget::makeItem(const QFileInfo &fi)
{
    QFileIconProvider iconProvider;
    QIcon icon = iconProvider.icon(fi);

    auto *item = new QTreeWidgetItem;

    // Column 0 – Name
    item->setIcon(0, icon);
    item->setText(0, fi.fileName());
    item->setToolTip(0, fi.filePath());

    // Column 1 – Type
    item->setText(1, mimeDescription(fi));
    item->setForeground(1, QApplication::palette().color(QPalette::PlaceholderText));

    // Column 2 – Size
    if (fi.isDir()) {
        item->setText(2, "—");
    } else {
        item->setText(2, formatSize(fi.size()));
    }
    item->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);

    // Column 3 – Modified
    item->setText(3, formatDate(fi.lastModified()));
    item->setForeground(3, QApplication::palette().color(QPalette::PlaceholderText));

    // Custom sort data: dirs before files (prefix "0:" / "1:")
    const QString sortKey = (fi.isDir() ? "0:" : "1:") + fi.fileName().toLower();
    item->setData(0, Qt::UserRole, sortKey);

    return item;
}

// ── Static utilities ──────────────────────────────────────────────────

QString FolderContentWidget::formatSize(qint64 bytes)
{
    if (bytes < 0)
        return "—";
    if (bytes == 0)
        return "0 B";

    static const char *units[] = { "B", "KB", "MB", "GB", "TB" };
    int idx = 0;
    double val = static_cast<double>(bytes);
    while (val >= 1024.0 && idx < 4) {
        val /= 1024.0;
        ++idx;
    }
    return idx == 0
           ? QString("%1 B").arg(bytes)
           : QString("%1 %2").arg(val, 0, 'f', val < 10 ? 2 : 1).arg(units[idx]);
}

QString FolderContentWidget::formatDate(const QDateTime &dt)
{
    if (!dt.isValid())
        return "—";

    const QDateTime now = QDateTime::currentDateTime();
    if (dt.date() == now.date())
        return tr("Today %1").arg(dt.toString("HH:mm"));
    if (dt.date() == now.date().addDays(-1))
        return tr("Yesterday %1").arg(dt.toString("HH:mm"));
    if (dt.date().year() == now.date().year())
        return dt.toString(tr("MM-dd HH:mm"));

    return dt.toString(tr("yyyy-MM-dd"));
}

QString FolderContentWidget::mimeDescription(const QFileInfo &fi)
{
    if (fi.isDir())
        return tr("Folder");

    static QMimeDatabase mimeDb;
    const QMimeType mt = mimeDb.mimeTypeForFile(fi,
                                                QMimeDatabase::MatchExtension);
    const QString desc = mt.comment();
    return desc.isEmpty() ? tr("File") : desc;
}
