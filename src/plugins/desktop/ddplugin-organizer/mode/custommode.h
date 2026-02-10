// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMMODE_H
#define CUSTOMMODE_H

#include "canvasorganizer.h"

namespace ddplugin_organizer {

class CustomModePrivate;
class CustomMode : public CanvasOrganizer
{
    Q_OBJECT
    friend class CustomModePrivate;

public:
    explicit CustomMode(QObject *parent = nullptr);
    ~CustomMode() override;
    OrganizerMode mode() const override;
    bool initialize(CollectionModel *) override;
    void reset() override;
    void layout() override;
    void detachLayout() override;
public slots:
    void rebuild();
    void onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl);
    void onFileInserted(const QModelIndex &parent, int first, int last);
    void onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
protected slots:
    bool filterDataRested(QList<QUrl> *urls) override;
    bool filterDataInserted(const QUrl &url) override;
    bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl) override;
    bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData) override;
private slots:
    void onNewCollection(const QList<QUrl> &);
    void onDeleteCollection(const QString &key);
    void onItemsChanged();

private:
    CustomModePrivate *d;
};

}

#endif   // CUSTOMMODE_H
