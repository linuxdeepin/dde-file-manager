// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCUSTOMACTIONPARSER_H
#define DCUSTOMACTIONPARSER_H

#include "dfmplugin_menu_global.h"
#include "dcustomactiondata.h"

#include <dfm-base/base/schemefactory.h>

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QIODevice>
#include <mutex>

namespace dfmplugin_menu {

class RegisterCustomFormat
{
public:
    static RegisterCustomFormat &instance();
    QSettings::Format customFormat();

private:
    RegisterCustomFormat();
    static bool readConf(QIODevice &device, QSettings::SettingsMap &settingsMap);
    static bool writeConf(QIODevice &device, const QSettings::SettingsMap &settingsMap);

private:
    QSettings::Format registeredcustomFormat;
};

class DCustomActionParser : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionParser(QObject *parent = nullptr);
    ~DCustomActionParser();

    QList<DCustomActionEntry> getActionFiles(bool onDesktop);

    inline void refresh()
    {
        actionEntry.clear();
        loadDir(menuPaths);
    }
protected slots:
    void delayRefresh();

signals:
    void customMenuChanged();

private:
    bool loadDir(const QStringList &dirPaths);
    bool parseFile(QSettings &actionSetting);
    bool parseFile(QList<DCustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const DCustomActionDefines::FileBasicInfos &basicInfos, bool &isSort, bool isTop = false);
    void initWatcher();
    void initHash();
    QVariant getValue(QSettings &actionSetting, const QString &group, const QString &key);
    bool actionFileInfos(DCustomActionDefines::FileBasicInfos &basicInfo, QSettings &actionSetting);

    void actionNameDynamicArg(DCustomActionData &act);
    void execDynamicArg(DCustomActionData &act);
    bool comboPosForTopAction(QSettings &actionSetting, const QString &group, DCustomActionData &act);
    static bool isActionShouldShow(const QStringList &notShowInList, bool onDesktop);

private:
    QTimer *refreshTimer = nullptr;
    QStringList menuPaths;
    QList<AbstractFileWatcherPointer> watcherGroup;
    QList<DCustomActionEntry> actionEntry;
    QSettings::Format customFormat;
    QHash<QString, DCustomActionDefines::ComboType> combos;
    QHash<QString, DCustomActionDefines::Separator> separtor;
    QHash<QString, DCustomActionDefines::ActionArg> actionNameArg;
    QHash<QString, DCustomActionDefines::ActionArg> actionExecArg;
    int hierarchyNum = 0;
    int topActionCount = 0;
};

}

#endif   // DCUSTOMACTIONPARSER_H
