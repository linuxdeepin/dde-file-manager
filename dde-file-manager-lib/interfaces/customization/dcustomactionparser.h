/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liqiang <liqianga@uniontech.com>
 *
 * Maintainer: liqiang <liqianga@uniontech.com>
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

#include "dcustomactiondata.h"

#include <QObject>
#include <QHash>
#include <QTimer>

class QSettings;
class QFileSystemWatcher;
class DCustomActionParser : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionParser(QObject *parent = nullptr);
    ~DCustomActionParser();

    bool loadDir(const QString &dirPath);
    QList<DCustomActionEntry> getActionFiles();

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

signals:
    void customMenuChanged();
public slots:
    void delayRefresh();
private:
    QTimer *m_refreshTimer = nullptr;
    QFileSystemWatcher  *m_fileWatcher  = nullptr;
    QList<DCustomActionEntry> m_actionEntry;
    QHash<QString, DCustomActionDefines::ComboType> m_combos;
    QHash<QString, DCustomActionDefines::Separator> m_separtor;
    QHash<QString, DCustomActionDefines::ActionArg> m_actionNameArg;
    QHash<QString, DCustomActionDefines::ActionArg> m_actionExecArg;
    int m_hierarchyNum = 0;
    int m_topActionCount = 0;
};

#endif // DCUSTOMACTIONPARSER_H
