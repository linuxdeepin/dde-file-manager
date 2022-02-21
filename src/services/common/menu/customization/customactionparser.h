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
#ifndef CUSTOMACTIONPARSER_H
#define CUSTOMACTIONPARSER_H
#include "customactiondata.h"

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QIODevice>
#include <QSettings>

class QFileSystemWatcher;
class RegisterCustomFormat
{
public:
    static RegisterCustomFormat &instance();
    QSettings::Format customConfFormat();

private:
    RegisterCustomFormat();
    static bool readConf(QIODevice &device, QSettings::SettingsMap &settingsMap);
    static bool writeConf(QIODevice &device, const QSettings::SettingsMap &settingsMap);

private:
    QSettings::Format customFormat;
};
class CustomActionParser : public QObject
{
    Q_OBJECT
public:
    explicit CustomActionParser(QObject *parent = nullptr);
    ~CustomActionParser();

    bool loadDir(const QString &dirPath);
    QList<CustomActionEntry> getActionFiles(bool onDesktop);

    bool parseFile(QSettings &actionSetting);
    bool parseFile(QList<CustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const CustomActionDefines::FileBasicInfos &basicInfos, bool &isSort, bool isTop = false);

    void initHash();
    QVariant getValue(QSettings &actionSetting, const QString &group, const QString &key);
    bool actionFileInfos(CustomActionDefines::FileBasicInfos &basicInfo, QSettings &actionSetting);

    void actionNameDynamicArg(CustomActionData &act);
    void execDynamicArg(CustomActionData &act);
    bool comboPosForTopAction(QSettings &actionSetting, const QString &group, CustomActionData &act);
    static bool isActionShouldShow(const QStringList &notShowInList, bool onDesktop);

signals:
    void customMenuChanged();
public slots:
    void delayRefresh();

private:
    QTimer *refreshTimer = nullptr;
    QFileSystemWatcher *fileWatcher = nullptr;
    QList<CustomActionEntry> actionEntry;
    QSettings::Format customFormat;
    QHash<QString, CustomActionDefines::ComboType> combos;
    QHash<QString, CustomActionDefines::Separator> separtor;
    QHash<QString, CustomActionDefines::ActionArg> actionNameArg;
    QHash<QString, CustomActionDefines::ActionArg> actionExecArg;
    int hierarchyNum = 0;
    int topActionCount = 0;
};

#endif   // CUSTOMACTIONPARSER_H
