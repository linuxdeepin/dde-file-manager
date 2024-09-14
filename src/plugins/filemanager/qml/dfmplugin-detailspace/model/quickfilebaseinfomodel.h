// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKFILEBASEINFOMODEL_H
#define QUICKFILEBASEINFOMODEL_H

#include "dfmplugin_detailspace_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QAbstractListModel>
#include <QUrl>

namespace dfmplugin_detailspace {

class QuickFileBaseInfoModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged FINAL)

public:
    enum Field {
        kKeyRole = Qt::UserRole + 1,
        kValueRole,
    };

    explicit QuickFileBaseInfoModel(QObject *parent = nullptr);

    QUrl url() const;
    Q_SLOT void setUrl(const QUrl &url);
    Q_SIGNAL void urlChanged(const QUrl &url);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void imageExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void videoExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void audioExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void receiveExtenInfo(DetailFilterType field, const QString &value);

private:
    void clearKeyValue();
    void resetFieldValue(const QUrl &url);
    void markFilter(const QUrl &url);
    void basicExpand(const QUrl &url);
    void basicFill(const QUrl &url);
    void sortKeyValue();
    void appendKeyValue(DetailFilterType field, const QString &value, const QString &specialKey = {}, bool insert = false);

    QString parseViewSize(const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &properties) const;
    QString parseDuration(const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &properties) const;
    QString displayFieldName(DetailFilterType field);

Q_SIGNALS:
    void sigExtenInfo(DetailFilterType field, const QString &value);

private:
    QUrl modelUrl;
    bool reseting = false;
    QFlags<DetailFilterType> markedType;   // 标记类型已被写入，非插入字段将跳过

    struct KeyValue
    {
        DetailFilterType type = DetailFilterType::kNotFilter;
        QString key;
        QString value;
    };
    QList<KeyValue> keyValueList;
};

}

#endif   // QUICKFILEBASEINFOMODEL_H
