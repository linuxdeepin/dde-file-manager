/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusfiledialoghandle.h"

#include <QMetaObject>
#include <QWindow>

#include <views/dfiledialog.h>

DBusFileDialogHandle::DBusFileDialogHandle(QWidget *parent)
    : DFileDialogHandle(parent)
{
    widget()->setAttribute(Qt::WA_NativeWindow);

    QWindow *window = widget()->windowHandle();

    if (window)
        connect(window, &QWindow::activeChanged, this, &DBusFileDialogHandle::windowActiveChanged);

    connect(&m_heartbeatTimer, &QTimer::timeout, this, &QObject::deleteLater);
    connect(widget(), &QWidget::destroyed, this, &QObject::deleteLater);
    connect(this, &DBusFileDialogHandle::currentUrlChanged, this, &DBusFileDialogHandle::directoryChanged);
    connect(this, &DBusFileDialogHandle::currentUrlChanged, this, &DBusFileDialogHandle::directoryUrlChanged);

    m_heartbeatTimer.setInterval(30 * 1000);
    m_heartbeatTimer.start();
}

DBusFileDialogHandle::~DBusFileDialogHandle()
{
    if (widget())
        widget()->close();
}

QString DBusFileDialogHandle::directory() const
{
    return DFileDialogHandle::directory().absolutePath();
}

void DBusFileDialogHandle::setDirectoryUrl(const QString &directory)
{
    QUrl dir(directory);
    if (dir.scheme().isEmpty()) {
        dir = QUrl::fromLocalFile(directory);
    }
    DFileDialogHandle::setDirectoryUrl(dir);
}

QString DBusFileDialogHandle::directoryUrl() const
{
    return DFileDialogHandle::directoryUrl().toString();
}

void DBusFileDialogHandle::selectUrl(const QString &url)
{
    DFileDialogHandle::selectUrl(QUrl(url));
}

QStringList DBusFileDialogHandle::selectedUrls() const
{
    QStringList list;

    for (const QUrl &url : DFileDialogHandle::selectedUrls())
        list << url.toString();

    return list;
}

int DBusFileDialogHandle::filter() const
{
    return DFileDialogHandle::filter();
}

void DBusFileDialogHandle::setFilter(int filters)
{
    DFileDialogHandle::setFilter(static_cast<QDir::Filters>(filters));
}

void DBusFileDialogHandle::setViewMode(int mode)
{
    DFileDialogHandle::setViewMode(static_cast<QFileDialog::ViewMode>(mode));
}

int DBusFileDialogHandle::viewMode() const
{
    return DFileDialogHandle::viewMode();
}

void DBusFileDialogHandle::setFileMode(int mode)
{
    DFileDialogHandle::setFileMode(static_cast<QFileDialog::FileMode>(mode));
}

void DBusFileDialogHandle::setAcceptMode(int mode)
{
    DFileDialogHandle::setAcceptMode(static_cast<QFileDialog::AcceptMode>(mode));
}

int DBusFileDialogHandle::acceptMode() const
{
    return DFileDialogHandle::acceptMode();
}

void DBusFileDialogHandle::setLabelText(int label, const QString &text)
{
    DFileDialogHandle::setLabelText(static_cast<QFileDialog::DialogLabel>(label), text);
}

QString DBusFileDialogHandle::labelText(int label) const
{
    return DFileDialogHandle::labelText(static_cast<QFileDialog::DialogLabel>(label));
}

void DBusFileDialogHandle::setOptions(int options)
{
    DFileDialogHandle::setOptions(static_cast<QFileDialog::Options>(options));
}

void DBusFileDialogHandle::setOption(int option, bool on)
{
    DFileDialogHandle::setOption(static_cast<QFileDialog::Option>(option), on);
}

int DBusFileDialogHandle::options() const
{
    return static_cast<int>(DFileDialogHandle::options());
}

bool DBusFileDialogHandle::testOption(int option) const
{
    return DFileDialogHandle::testOption(static_cast<QFileDialog::Option>(option));
}

WId DBusFileDialogHandle::winId() const
{
    return widget()->winId();
}

void DBusFileDialogHandle::setWindowTitle(const QString &title)
{
    widget()->setWindowTitle(title);
}

bool DBusFileDialogHandle::windowActive() const
{
    return widget()->isActiveWindow();
}

void DBusFileDialogHandle::activateWindow()
{
    widget()->activateWindow();
}

int DBusFileDialogHandle::heartbeatInterval() const
{
    return m_heartbeatTimer.interval();
}

void DBusFileDialogHandle::makeHeartbeat()
{
    m_heartbeatTimer.start();
}

quint32 DBusFileDialogHandle::windowFlags() const
{
    return widget()->windowFlags();
}

void DBusFileDialogHandle::setHeartbeatInterval(int heartbeatInterval)
{
    m_heartbeatTimer.setInterval(heartbeatInterval);
}

void DBusFileDialogHandle::setWindowFlags(quint32 windowFlags)
{
    widget()->setWindowFlags(static_cast<Qt::WindowFlags>(windowFlags));
}
