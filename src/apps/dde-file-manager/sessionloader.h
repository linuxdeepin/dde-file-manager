// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USMSESSIONLOADER_H
#define USMSESSIONLOADER_H

#include <QLibrary>
#include <QObject>
#include <QUrl>
#include <QMap>

typedef int (*FnConnectSM)(unsigned long long wid);
typedef void (*FnDisconnectSM)(void);
typedef void (*FnRequestSaveYourself)(void);
typedef int (*FnParseArguments)(int argc, char *argv[]);
typedef int (*FnSetWindowProperty)(unsigned long long wid);
typedef const char *(*FnFilename)(unsigned long long wid);

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
    void process(const QStringList &args);
    bool readPath(const QString &fileName, QString *path = nullptr);

private:
    explicit SessionBusiness(QObject *parent = nullptr);
    virtual ~SessionBusiness();

    void bindEvents();
    void connectToUsmSever(quint64 wid);
    void savePath(quint64 wid, const QString &path);
    char **parseArguments(int &argc);
    void releaseArguments(int argc, char **argv_new);

private slots:
    void onWindowOpened(quint64 windId);
    void onWindowClosed(quint64 windId);
    void onCurrentUrlChanged(quint64 windId, const QUrl &url);

private:
    UsmSessionAPI sessionAPI;
    QStringList arguments;
    QMap<quint64, QString> windowStatus;
};

#endif   // USMSESSIONLOADER_H
