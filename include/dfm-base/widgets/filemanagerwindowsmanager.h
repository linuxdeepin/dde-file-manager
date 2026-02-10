// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERWINDOWSMANAGER_H
#define FILEMANAGERWINDOWSMANAGER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <QObject>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

class FileManagerWindowsManagerPrivate;
class FileManagerWindowsManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileManagerWindowsManager)

public:
    using FMWindow = DFMBASE_NAMESPACE::FileManagerWindow;
    using WindowCreator = std::function<FMWindow *(const QUrl &)>;

public:
    static FileManagerWindowsManager &instance();

    void setCustomWindowCreator(WindowCreator creator);
    FMWindow *createWindow(const QUrl &url, bool isNewWindow = true, QString *errorString = nullptr);
    void showWindow(FMWindow *window);
    quint64 findWindowId(const QWidget *window);
    FMWindow *findWindowById(quint64 winId);
    QList<quint64> windowIdList();
    void resetPreviousActivedWindowId();
    quint64 previousActivedWindowId();
    quint64 lastActivedWindowId();
    bool containsCurrentUrl(const QUrl &url, const QWidget *win = nullptr);

Q_SIGNALS:
    void windowCreated(quint64 windId);
    void windowOpened(quint64 windId);
    void windowClosed(quint64 windId);
    void lastWindowClosed(quint64 winId);

    void currentUrlChanged(quint64 windId, const QUrl &url);

private:
    explicit FileManagerWindowsManager(QObject *parent = nullptr);
    ~FileManagerWindowsManager() override;

private:
    QScopedPointer<FileManagerWindowsManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#define FMWindowsIns DFMBASE_NAMESPACE::FileManagerWindowsManager::instance()

#endif   // FILEMANAGERWINDOWSMANAGER_H
