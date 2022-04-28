/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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

#ifndef DCUSTOMACTIONPARSER_H
#define DCUSTOMACTIONPARSER_H

#include "dfmplugin_menu_global.h"
#include "dcustomactiondata.h"

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QIODevice>
#include <QSettings>

class QFileSystemWatcher;

DPMENU_BEGIN_NAMESPACE

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
    static DCustomActionParser *instance();
    ~DCustomActionParser();

    QList<DCustomActionEntry> getActionFiles(bool onDesktop);

public slots:
    void delayRefresh();

signals:
    void customMenuChanged();

private:
    bool loadDir(const QStringList &dirPaths);
    bool parseFile(QSettings &actionSetting);
    bool parseFile(QList<DCustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const DCustomActionDefines::FileBasicInfos &basicInfos, bool &isSort, bool isTop = false);

    void initHash();
    QVariant getValue(QSettings &actionSetting, const QString &group, const QString &key);
    bool actionFileInfos(DCustomActionDefines::FileBasicInfos &basicInfo, QSettings &actionSetting);

    void actionNameDynamicArg(DCustomActionData &act);
    void execDynamicArg(DCustomActionData &act);
    bool comboPosForTopAction(QSettings &actionSetting, const QString &group, DCustomActionData &act);
    static bool isActionShouldShow(const QStringList &notShowInList, bool onDesktop);

protected:
    explicit DCustomActionParser(QObject *parent = nullptr);

private:
    QTimer *refreshTimer = nullptr;
    QStringList menuPaths;
    QFileSystemWatcher *fileWatcher = nullptr;
    QList<DCustomActionEntry> actionEntry;
    QSettings::Format customFormat;
    QHash<QString, DCustomActionDefines::ComboType> combos;
    QHash<QString, DCustomActionDefines::Separator> separtor;
    QHash<QString, DCustomActionDefines::ActionArg> actionNameArg;
    QHash<QString, DCustomActionDefines::ActionArg> actionExecArg;
    int hierarchyNum = 0;
    int topActionCount = 0;
};

#define CustomParserIns DPMENU_NAMESPACE::DCustomActionParser::instance()

DPMENU_END_NAMESPACE

#endif   // DCUSTOMACTIONPARSER_H
