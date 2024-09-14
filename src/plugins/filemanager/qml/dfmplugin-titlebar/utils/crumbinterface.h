// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

    void setKeepAddressBar(bool keep);
    void setSupportedScheme(const QString &scheme);
    bool isKeepAddressBar();
    bool isSupportedScheme(const QString &scheme);

    void processAction(ActionType type);
    void crumbUrlChangedBehavior(const QUrl &url);
    FAKE_VIRTUAL QList<CrumbData> seprateUrl(const QUrl &url);
    void requestCompletionList(const QUrl &url);
    void cancelCompletionListTransmission();

signals:
    void hideAddressBar(bool cd);
    void pauseSearch();
    void keepAddressBar(const QUrl &url);
    void hideAddrAndUpdateCrumbs(const QUrl &url);
    void completionFound(const QStringList &completions);   //< emit multiple times with less or equials to 10 items in a group.
    void completionListTransmissionCompleted();   //< emit when all avaliable completions has been sent.

private slots:
    void onUpdateChildren(QList<QUrl> children);

private:
    QString curScheme;
    bool keepAddr { false };
    QPointer<DFMBASE_NAMESPACE::TraversalDirThread> folderCompleterJobPointer;
};

}

#endif   // CRUMBINTERFACE_H
