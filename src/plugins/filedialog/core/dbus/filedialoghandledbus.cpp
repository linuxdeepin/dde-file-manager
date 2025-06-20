// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialoghandledbus.h"

#include <QMetaObject>
#include <QWindow>

FileDialogHandleDBus::FileDialogHandleDBus(QWidget *parent)
    : FileDialogHandle(parent)
{
    widget()->setAttribute(Qt::WA_NativeWindow);

    QWindow *window = widget()->windowHandle();

    if (window)
        connect(window, &QWindow::activeChanged, this, &FileDialogHandleDBus::windowActiveChanged);

    connect(&curHeartbeatTimer, &QTimer::timeout, this, &QObject::deleteLater);
    connect(widget(), &QWidget::destroyed, this, &QObject::deleteLater);
    connect(this, &FileDialogHandleDBus::currentUrlChanged, this, &FileDialogHandleDBus::directoryChanged);
    connect(this, &FileDialogHandleDBus::currentUrlChanged, this, &FileDialogHandleDBus::directoryUrlChanged);

    curHeartbeatTimer.setInterval(30 * 1000);
    curHeartbeatTimer.start();
}

FileDialogHandleDBus::~FileDialogHandleDBus()
{
    if (widget())
        widget()->close();
}

QString FileDialogHandleDBus::directory() const
{
    return FileDialogHandle::directory().absolutePath();
}

void FileDialogHandleDBus::setDirectoryUrl(const QString &directory)
{
    QString path = QDir::homePath();
    if(!directory.isEmpty())
       path = directory;
    QUrl dir(path);
    if (dir.scheme().isEmpty()) {
        dir = QUrl::fromLocalFile(path);
    }
    FileDialogHandle::setDirectoryUrl(dir);
}

QString FileDialogHandleDBus::directoryUrl() const
{
    return FileDialogHandle::directoryUrl().toString();
}

void FileDialogHandleDBus::selectUrl(const QString &url)
{
    FileDialogHandle::selectUrl(QUrl(url));
}

QStringList FileDialogHandleDBus::selectedUrls() const
{
    QStringList list;

    for (const QUrl &url : FileDialogHandle::selectedUrls())
        list << url.toString();

    return list;
}

int FileDialogHandleDBus::filter() const
{
    return FileDialogHandle::filter();
}

void FileDialogHandleDBus::setFilter(int filters)
{
    FileDialogHandle::setFilter(static_cast<QDir::Filters>(filters));
}

void FileDialogHandleDBus::setViewMode(int mode)
{
    FileDialogHandle::setViewMode(static_cast<QFileDialog::ViewMode>(mode));
}

int FileDialogHandleDBus::viewMode() const
{
    return FileDialogHandle::viewMode();
}

void FileDialogHandleDBus::setFileMode(int mode)
{
    FileDialogHandle::setFileMode(static_cast<QFileDialog::FileMode>(mode));
}

void FileDialogHandleDBus::setAcceptMode(int mode)
{
    FileDialogHandle::setAcceptMode(static_cast<QFileDialog::AcceptMode>(mode));
}

int FileDialogHandleDBus::acceptMode() const
{
    return FileDialogHandle::acceptMode();
}

void FileDialogHandleDBus::setLabelText(int label, const QString &text)
{
    FileDialogHandle::setLabelText(static_cast<QFileDialog::DialogLabel>(label), text);
}

QString FileDialogHandleDBus::labelText(int label) const
{
    return FileDialogHandle::labelText(static_cast<QFileDialog::DialogLabel>(label));
}

void FileDialogHandleDBus::setOptions(int options)
{
    FileDialogHandle::setOptions(static_cast<QFileDialog::Options>(options));
}

void FileDialogHandleDBus::setOption(int option, bool on)
{
    FileDialogHandle::setOption(static_cast<QFileDialog::Option>(option), on);
}

int FileDialogHandleDBus::options() const
{
    return static_cast<int>(FileDialogHandle::options());
}

bool FileDialogHandleDBus::testOption(int option) const
{
    return FileDialogHandle::testOption(static_cast<QFileDialog::Option>(option));
}

qulonglong FileDialogHandleDBus::winId() const
{
    return FileDialogHandle::winId();
}

void FileDialogHandleDBus::setWindowTitle(const QString &title)
{
    widget()->setWindowTitle(title);
}

bool FileDialogHandleDBus::windowActive() const
{
    return widget() ? widget()->isActiveWindow() : false;
}

void FileDialogHandleDBus::activateWindow()
{
    if (widget())
        widget()->activateWindow();
}

int FileDialogHandleDBus::heartbeatInterval() const
{
    return curHeartbeatTimer.interval();
}

void FileDialogHandleDBus::makeHeartbeat()
{
    curHeartbeatTimer.start();
}

quint32 FileDialogHandleDBus::windowFlags() const
{
    return widget()->windowFlags();
}

void FileDialogHandleDBus::setHeartbeatInterval(int interval)
{
    curHeartbeatTimer.setInterval(interval);
}

void FileDialogHandleDBus::setWindowFlags(quint32 windowFlags)
{
    widget()->setWindowFlags(static_cast<Qt::WindowFlags>(windowFlags));
}
