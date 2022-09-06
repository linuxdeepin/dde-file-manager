// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSFILEDIALOGHANDLE_H
#define DBUSFILEDIALOGHANDLE_H

#include "dfiledialoghandle.h"

#include <QTimer>

class DBusFileDialogHandle : public DFileDialogHandle
{
    Q_OBJECT

    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
    Q_PROPERTY(QString directoryUrl READ directoryUrl WRITE setDirectoryUrl NOTIFY directoryUrlChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(int filter READ filter WRITE setFilter)
    Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(int acceptMode READ acceptMode WRITE setAcceptMode)
    Q_PROPERTY(bool windowActive READ windowActive NOTIFY windowActiveChanged)
    Q_PROPERTY(int heartbeatInterval READ heartbeatInterval WRITE setHeartbeatInterval)
    Q_PROPERTY(quint32 windowFlags READ windowFlags WRITE setWindowFlags)
    Q_PROPERTY(bool hideOnAccept READ hideOnAccept WRITE setHideOnAccept)

public:
    explicit DBusFileDialogHandle(QWidget *parent = nullptr);
    virtual ~DBusFileDialogHandle();

    QString directory() const;

    void setDirectoryUrl(const QString & directory);
    QString directoryUrl() const;

    void selectUrl(const QString &url);
    QStringList selectedUrls() const;

    int filter() const;
    void setFilter(int filters);

    void setViewMode(int mode);
    int viewMode() const;

    void setFileMode(int mode);

    void setAcceptMode(int mode);
    int acceptMode() const;

    void setLabelText(int label, const QString &text);
    QString labelText(int label) const;

    void setOptions(int options);
    void setOption(int option, bool on = true);
    int options() const;
    bool testOption(int option) const;

    WId winId() const;
    void setWindowTitle(const QString &title);
    bool windowActive() const;

    void activateWindow();

    int heartbeatInterval() const;
    void makeHeartbeat();

    quint32 windowFlags() const;

public slots:
    void setHeartbeatInterval(int heartbeatInterval);
    void setWindowFlags(quint32 windowFlags);

signals:
    void windowActiveChanged();
    void directoryChanged();
    void directoryUrlChanged();

private:
    QTimer m_heartbeatTimer;
};

#endif // DBUSFILEDIALOGHANDLE_H
