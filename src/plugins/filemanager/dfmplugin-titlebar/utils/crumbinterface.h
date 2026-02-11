// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRUMBINTERFACE_H
#define CRUMBINTERFACE_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/utils/traversaldirthread.h>

#include <QObject>
#include <QPointer>

namespace dfmplugin_titlebar {

#define FAKE_VIRTUAL

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

    void setSupportedScheme(const QString &scheme);
    bool isSupportedScheme(const QString &scheme);

    void processAction(ActionType type);
    FAKE_VIRTUAL QList<CrumbData> seprateUrl(const QUrl &url);
    void requestCompletionList(const QUrl &url);
    void cancelCompletionListTransmission();

signals:
    void hideAddressBar();
    void pauseSearch();
    void keepAddressBar(const QUrl &url);
    void hideAddrAndUpdateCrumbs(const QUrl &url);
    void completionFound(const QStringList &completions);   //< emit multiple times with less or equials to 10 items in a group.
    void completionListTransmissionCompleted();   //< emit when all avaliable completions has been sent.

private slots:
    void onUpdateChildren(QList<QUrl> children);

private:
    QString curScheme;
    QPointer<DFMBASE_NAMESPACE::TraversalDirThread> folderCompleterJobPointer;
};

}

#endif   // CRUMBINTERFACE_H
