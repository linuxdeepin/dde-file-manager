/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef CRUMBINTERFACE_H
#define CRUMBINTERFACE_H

#include "dfmplugin_titlebar_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QObject>
#include <QPointer>

DPTITLEBAR_BEGIN_NAMESPACE

#define FAKE_VIRTUAL

using DSB_FM_NAMESPACE::TitleBar::CrumbData;
using DSB_FM_NAMESPACE::TitleBar::seprateUrlCallback;
using DSB_FM_NAMESPACE::TitleBar::supportedUrlCallback;

class CrumbInterface final : public QObject
{
    Q_OBJECT
public:
    enum ActionType {
        kEscKeyPressed,
        kClearButtonPressed,
        kAddressBarLostFocus
    };

    explicit CrumbInterface(QObject *parent = nullptr);

    void setKeepAddressBar(bool keep);
    void processAction(ActionType type);
    void crumbUrlChangedBehavior(const QUrl &url);
    FAKE_VIRTUAL bool supportedUrl(const QUrl &url);
    FAKE_VIRTUAL QList<CrumbData> seprateUrl(const QUrl &url);
    void requestCompletionList(const QUrl &url);
    void cancelCompletionListTransmission();

    void registewrSupportedUrlCallback(const supportedUrlCallback &func);
    void registerSeprateUrlCallback(const seprateUrlCallback &func);

signals:
    void hideAddressBar(bool cd);
    void pauseSearch();
    void keepAddressBar(const QUrl &url);
    void hideAddrAndUpdateCrumbs(const QUrl &url);
    void completionFound(const QStringList &completions);   //< emit multiple times with less or equials to 10 items in a group.
    void completionListTransmissionCompleted();   //< emit when all avaliable completions has been sent.

private slots:
    void onUpdateChildren(const QList<QUrl> &urlList);

private:
    bool keepAddr { false };
    supportedUrlCallback supportedUrlFunc;
    seprateUrlCallback seprateUrlFunc;
    QPointer<DFMBASE_NAMESPACE::TraversalDirThread> folderCompleterJobPointer;
};

DPTITLEBAR_END_NAMESPACE

#endif   // CRUMBINTERFACE_H
