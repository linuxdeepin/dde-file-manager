// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALENTRYMENUSCENE_P_H
#define VIRTUALENTRYMENUSCENE_P_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DPSMBBROWSER_BEGIN_NAMESPACE

class VirtualEntryMenuScene;
class VirtualEntryMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class VirtualEntryMenuScene;

public:
    explicit VirtualEntryMenuScenePrivate(VirtualEntryMenuScene *qq);
    void setActionVisible(const QStringList &visibleActions, QMenu *menu);
    void insertActionBefore(const QString &inserted, const QString &after, QMenu *menu);

    // menu action trigger handlers
    void hookCptActions(QAction *triggered);
    void actUnmountAggregatedItem(bool removeEntry = false);
    void actForgetAggregatedItem();
    void actMountSeperatedItem();
    void actRemoveVirtualEntry();
    void actCptMount();
    void actCptForget();

    void gotoDefaultPageOnUnmount();
    static void tryRemoveAggregatedEntry(const QString &stdSmb, const QString &stdSmbSharePath);

private:
    bool aggregatedEntrySelected { false };
    bool seperatedEntrySelected { false };
    QString stdSmb;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // VIRTUALENTRYMENUSCENE_P_H
