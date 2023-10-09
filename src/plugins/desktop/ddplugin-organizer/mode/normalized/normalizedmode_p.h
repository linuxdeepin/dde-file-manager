// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALIZEDMODE_P_H
#define NORMALIZEDMODE_P_H

#include "mode/normalizedmode.h"
#include "fileclassifier.h"
#include "collection/collectionholder.h"
#include "normalizedmodebroker.h"
#include "models/itemselectionmodel.h"
#include "mode/selectionsynchelper.h"

namespace ddplugin_organizer {

class NormalizedModePrivate : public QObject
{
    Q_OBJECT
public:
    explicit NormalizedModePrivate(NormalizedMode *qq);
    ~NormalizedModePrivate();

    QPoint findValidPos(QPoint &nextPos, int &currentIndex, CollectionStyle &style, const int width, const int height);

    void collectionStyleChanged(const QString &id);
    CollectionHolderPointer createCollection(const QString &id);
    void switchCollection();
    void openEditor(const QUrl &url);
    void checkTouchFile(const QUrl &url);
    void checkPastedFiles(const QList<QUrl> &urls);
public slots:
    void onSelectFile(QList<QUrl> &urls, int flag);
    void onClearSelection();
    void onDropFile(const QString &collection, QList<QUrl> &urls);
public:
    void restore(const QList<CollectionBaseDataPtr> &cfgs);
    FileClassifier *classifier = nullptr;
    QHash<QString, CollectionHolderPointer> holders;
    NormalizedModeBroker *broker = nullptr;
    ItemSelectionModel *selectionModel = nullptr;
    SelectionSyncHelper *selectionHelper = nullptr;
private:
    NormalizedMode *q;
};

}

#endif // NORMALIZEDMODE_P_H
