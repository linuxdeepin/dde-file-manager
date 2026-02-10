// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOG_P_H
#define FILEDIALOG_P_H

#include "filedialogplugin_core_global.h"
#include "filedialogstatusbar.h"
#include <dfm-base/interfaces/abstractframe.h>

#include <QObject>
#include <QEventLoop>
#include <QFileDialog>
#include <QRegularExpression>

#include <linux/limits.h>
#include <mutex>

DFMBASE_USE_NAMESPACE

namespace filedialog_core {

class FileDialog;
class FileDialogPrivate : public QObject
{
    Q_OBJECT
    friend class FileDialog;
    FileDialog *const q;

public:
    explicit FileDialogPrivate(FileDialog *qq);
    ~FileDialogPrivate();

    void handleSaveAcceptBtnClicked();
    void handleOpenAcceptBtnClicked();
    void handleOpenNewWindow(const QUrl &url);
    bool checkFileSuffix(const QString &filename, QString &suffix);
    void setLastVisited(const QUrl &dir);

public Q_SLOTS:
    void saveLastVisited();

private:
    static constexpr int kDefaultWindowWidth { 960 };
    static constexpr int kDefaultWindowHeight { 540 };

    bool isFileView { false };
    bool lastIsFileView { false };
    bool hideOnAccept { true };
    FileDialogStatusBar *statusBar { nullptr };
    QEventLoop *eventLoop { nullptr };
    QFileDialog::FileMode fileMode { QFileDialog::AnyFile };
    QFileDialog::AcceptMode acceptMode { QFileDialog::AcceptOpen };
    bool acceptCanOpenOnSave { false };
    QStringList nameFilters;
    QDir::Filters filters { QDir::NoFilter };
    int currentNameFilterIndex { -1 };
    QString currentInputName;
    bool allowMixedSelection { false };
    QFileDialog::Options options;
    QUrl currentUrl;
    QUrl lastVisitedDir;
    QTimer *delaySaveTimer { nullptr };

    static QStringList cleanFilterList(const QString &filter)
    {
        static const QString filterRegExp = QStringLiteral("^(.*)\\(([a-zA-Z0-9_.,*? +;#\\-\\[\\]@\\{\\}/!<>\\\\]*)\\)$");
        static const QRegularExpression regexp(filterRegExp);
        QString f = filter.trimmed();

        QRegularExpressionMatch match = regexp.match(f);
        if (match.hasMatch()) {
            QString nameFilter = match.captured(2);
            return nameFilter.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        }
        return QStringList(f);
    }
};

}

#endif   // FILEDIALOG_P_H
