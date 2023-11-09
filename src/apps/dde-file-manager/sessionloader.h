// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USMSESSIONLOADER_H
#define USMSESSIONLOADER_H

#include <QLibrary>
#include <QObject>
#include <QUrl>

typedef int (*FnConnectSM)(void);
typedef void (*FnDisconnectSM)(void);
typedef void (*FnRequestSaveYourself)(void);
typedef int (*FnParseArguments)(int argc, char *argv[]);
typedef int (*FnSetWindowProperty)(unsigned long long wid);
typedef const char *(*FnFilename)(void);

class UsmSessionAPI
{
public:
    UsmSessionAPI();
    ~UsmSessionAPI();

public:
    bool init();
    bool isInitialized() const;

public:
    FnConnectSM connectSM { nullptr };
    FnDisconnectSM disconnectSM { nullptr };
    FnRequestSaveYourself requestSaveYourself { nullptr };
    FnParseArguments parseArguments { nullptr };
    FnSetWindowProperty setWindowProperty { nullptr };
    FnFilename filename { nullptr };

private:
    bool initialized { false };
    QLibrary libUsm;
};

class SessionBusiness final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionBusiness)

public:
    static SessionBusiness *instance();
    UsmSessionAPI *getAPI();
    bool readPath(QString *path = nullptr);

private:
    explicit SessionBusiness(QObject *parent = nullptr);
    virtual ~SessionBusiness();

    void bindEvents();
    void savePath(const QString &path);

private slots:
    void onWindowOpened(quint64 windId);
    void onCurrentUrlChanged(quint64 windId, const QUrl &url);

private:
    UsmSessionAPI sessionAPI;
};

#endif   // USMSESSIONLOADER_H
