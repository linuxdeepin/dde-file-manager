// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMHELPER_H
#define EMBLEMHELPER_H

#include "dfmplugin_emblem_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-framework/dpf.h>

#include <QIcon>
#include <QThread>
#include <QSet>

DPEMBLEM_BEGIN_NAMESPACE
using Product = QList<QIcon>;   // for a url
using ProductQueue = QHash<QUrl, Product>;

class GioEmblemWorker : public QObject
{
    Q_OBJECT

public:
    QList<QIcon> fetchEmblems(const FileInfoPointer &info) const;

public Q_SLOTS:
    void onProduce(const FileInfoPointer &info);
    void onClear();

Q_SIGNALS:
    void emblemChanged(const QUrl &url, const Product &product);

private:
    QMap<int, QIcon> getGioEmblems(const FileInfoPointer &info) const;
    bool parseEmblemString(QIcon *emblem, QString &pos, const QString &emblemStr) const;
    bool iconNamesEqual(const QList<QIcon> &first, const QList<QIcon> &second);
    void setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> *iconMap) const;

private:
    ProductQueue cache;
};

class EmblemHelper : public QObject
{
    Q_OBJECT

public:
    explicit EmblemHelper(QObject *parent);
    ~EmblemHelper() override;

    inline bool hasEmblem(const QUrl &url) const { return productQueue.contains(url); }
    inline void clearEmblem() { productQueue.clear(); }

    QList<QIcon> systemEmblems(const FileInfoPointer &info) const;
    QList<QRectF> emblemRects(const QRectF &paintArea) const;
    QList<QIcon> gioEmblemIcons(const QUrl &url) const;
    void pending(const FileInfoPointer &info);
    bool isExtEmblemProhibited(const FileInfoPointer &info, const QUrl &url);

Q_SIGNALS:
    void requestProduce(const FileInfoPointer &info);
    void requestClear();

private Q_SLOTS:
    void onEmblemChanged(const QUrl &url, const Product &product);
    bool onUrlChanged(quint64 windowId, const QUrl &url);

private:
    void initialize();
    QIcon standardEmblem(const SystemEmblemType type) const;

private:
    GioEmblemWorker *worker { new GioEmblemWorker };
    ProductQueue productQueue;
    QThread workerThread;
};

DPEMBLEM_END_NAMESPACE

Q_DECLARE_METATYPE(DPEMBLEM_NAMESPACE::Product);
Q_DECLARE_METATYPE(DPEMBLEM_NAMESPACE::ProductQueue);

#endif   // EMBLEMHELPER_H
