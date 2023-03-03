// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMHELPER_H
#define EMBLEMHELPER_H

#include "dfmplugin_emblem_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-framework/dpf.h>

#include <QIcon>
#include <QThread>

DPEMBLEM_BEGIN_NAMESPACE
using Product = QList<QIcon>;   // for a url
using ProductQueue = QHash<QUrl, Product>;

class EmblemWorker : public QObject
{
    Q_OBJECT

public:
    QList<QIcon> fetchEmblems(const QUrl &url) const;

public Q_SLOTS:
    void onProduce(const QUrl &url);
    void onClear();

Q_SIGNALS:
    void emblemChanged(const QUrl &url, const Product &product);

private:
    QList<QIcon> getSystemEmblems(const AbstractFileInfoPointer &info) const;
    QMap<int, QIcon> getGioEmblems(const AbstractFileInfoPointer &info) const;
    QIcon standardEmblem(const SystemEmblemType type) const;
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

    QList<QRectF> emblemRects(const QRectF &paintArea) const;
    QList<QIcon> emblemIcons(const QUrl &url) const;
    void pending(const QUrl &url);

Q_SIGNALS:
    void requestProduce(const QUrl &url);
    void requestClear();

private Q_SLOTS:
    void onEmblemChanged(const QUrl &url, const Product &product);
    bool onUrlChanged(quint64 windowId, const QUrl &url);

private:
    void initialize();

private:
    EmblemWorker *worker { new EmblemWorker };
    ProductQueue productQueue;
    QThread workerThread;

    static constexpr int kProduceInterval { 300 };   // ms
};

DPEMBLEM_END_NAMESPACE

Q_DECLARE_METATYPE(DPEMBLEM_NAMESPACE::Product);
Q_DECLARE_METATYPE(DPEMBLEM_NAMESPACE::ProductQueue);

#endif   // EMBLEMHELPER_H
