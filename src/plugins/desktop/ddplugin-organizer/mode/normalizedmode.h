// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALIZEDMODE_H
#define NORMALIZEDMODE_H

#include "canvasorganizer.h"

#include <QSharedPointer>

namespace ddplugin_organizer {
class NormalizedModePrivate;
class NormalizedMode : public CanvasOrganizer
{
    Q_OBJECT
    friend class NormalizedModePrivate;
    friend class NormalizedModeBroker;

public:
    explicit NormalizedMode(QObject *parent = nullptr);
    ~NormalizedMode() override;
    OrganizerMode mode() const override;
    bool initialize(CollectionModel *) override;
    void reset() override;
    void layout() override;
    void detachLayout() override;

public slots:
    void rebuild(bool reorganize = false);
    void onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl);
    void onFileInserted(const QModelIndex &parent, int first, int last);
    void onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onReorganizeDesktop();

protected slots:
    bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint) override;
    bool filterDataRested(QList<QUrl> *urls) override;
    bool filterDataInserted(const QUrl &url) override;
    bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl) override;
    bool filterShortcutkeyPress(int viewIndex, int key, int modifiers) const override;

    void onCollectionEditStatusChanged(bool editing);
    void changeCollectionSurface(const QString &screenName);
    void deactiveAllPredictors();

protected:
    bool setClassifier(Classifier id);
    void removeClassifier();

private:
    NormalizedModePrivate *d = nullptr;
};

}

#endif   // NORMALIZEDMODE_H
