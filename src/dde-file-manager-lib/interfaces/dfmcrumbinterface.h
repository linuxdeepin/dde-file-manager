/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFMCRUMBINTERFACE_H
#define DFMCRUMBINTERFACE_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class CrumbData
{
public:
    CrumbData(DUrl url = DUrl(), QString displayText = QString(), QString iconName = QString(), QString iconKey = "icon");

    operator QString() const;

    DUrl url = DUrl();
    QString iconName = QString(); // icon group name
    QString displayText = QString();
    QString iconKey = "icon"; // icon key
};

class DFMCrumbBar;
class DFMCrumbInterfacePrivate;
class DFMCrumbInterface : public QObject
{
    Q_OBJECT
public:
    explicit DFMCrumbInterface(QObject *parent = 0);
    ~DFMCrumbInterface();

    // Actions may cause DFMAddressBar focusOut
    enum ActionType {
        EscKeyPressed,
        ClearButtonPressed,
        AddressBarLostFocus
    };

    virtual void processAction(ActionType type);
    virtual void crumbUrlChangedBehavior(const DUrl url);
    virtual DFMCrumbBar* crumbBar() final;
    virtual void setCrumbBar(DFMCrumbBar *crumbBar) final;
    virtual bool supportedUrl(DUrl) = 0;
    virtual QList<CrumbData> seprateUrl(const DUrl &url);
    virtual void requestCompletionList(const DUrl &url);
    virtual void cancelCompletionListTransmission();

signals:
    void completionFound(const QStringList &completions); //< emit multiple times with less or equials to 10 items in a group.
    void completionListTransmissionCompleted(); //< emit when all avaliable completions has been sent.

private:
    QScopedPointer<DFMCrumbInterfacePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMCrumbInterface)
};

DFM_END_NAMESPACE

#endif // DFMCRUMBINTERFACE_H
