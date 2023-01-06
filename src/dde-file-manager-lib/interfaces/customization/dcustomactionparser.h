// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCUSTOMACTIONPARSER_H
#define DCUSTOMACTIONPARSER_H

#include "dcustomactiondata.h"

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QIODevice>
#include <QSettings>

class QFileSystemWatcher;
class RegisterCustomFormat
{
public:
    static RegisterCustomFormat& instance();
    QSettings::Format customFormat();
private:
    RegisterCustomFormat();
    static bool readConf(QIODevice &device, QSettings::SettingsMap &settingsMap);
    static bool writeConf(QIODevice &device, const QSettings::SettingsMap &settingsMap);
private:
    QSettings::Format m_customFormat;
};
class DCustomActionParser : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionParser(QObject *parent = nullptr);
    ~DCustomActionParser();

    bool loadDir(const QString &dirPath);
    QList<DCustomActionEntry> getActionFiles(bool onDesktop);

    bool parseFile(QSettings &actionSetting);
    bool parseFile(QList<DCustomActionData> &childrenActions
                   , QSettings &actionSetting
                   , const QString &group
                   , const DCustomActionDefines::FileBasicInfos& basicInfos
                   , bool &isSort
                   , bool isTop = false);


    void initHash();
    QVariant getValue(QSettings &actionSetting, const QString &group, const QString &key);
    bool actionFileInfos(DCustomActionDefines::FileBasicInfos &basicInfo, QSettings &actionSetting);

    void actionNameDynamicArg(DCustomActionData &act);
    void execDynamicArg(DCustomActionData &act);
    bool comboPosForTopAction(QSettings &actionSetting, const QString&group, DCustomActionData &act);
    static bool isActionShouldShow(const QStringList &notShowInList, bool onDesktop);

signals:
    void customMenuChanged();
public slots:
    void delayRefresh();
private:
    QTimer *m_refreshTimer = nullptr;
    QFileSystemWatcher  *m_fileWatcher  = nullptr;
    QList<DCustomActionEntry> m_actionEntry;
    QSettings::Format m_customFormat;
    QHash<QString, DCustomActionDefines::ComboType> m_combos;
    QHash<QString, DCustomActionDefines::Separator> m_separtor;
    QHash<QString, DCustomActionDefines::ActionArg> m_actionNameArg;
    QHash<QString, DCustomActionDefines::ActionArg> m_actionExecArg;
    int m_hierarchyNum = 0;
    int m_topActionCount = 0;
};

#endif // DCUSTOMACTIONPARSER_H
