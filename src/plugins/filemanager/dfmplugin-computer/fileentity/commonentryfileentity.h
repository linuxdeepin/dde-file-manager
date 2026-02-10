// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONENTRYFILEENTITY_H
#define COMMONENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmplugin_computer {

class CommonEntryFileEntity final : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT

public:
    explicit CommonEntryFileEntity(const QUrl &url);
    ~CommonEntryFileEntity() override;

    QString displayName() const override;
    QIcon icon() const override;
    bool exists() const override;

    bool showProgress() const override;
    bool showTotalSize() const override;
    bool showUsageSize() const override;
    EntryOrder order() const override;

    void refresh() override;
    quint64 sizeTotal() const override;
    quint64 sizeUsage() const override;
    QString description() const override;
    QUrl targetUrl() const override;
    bool isAccessable() const override;
    bool renamable() const override;
    QVariantHash extraProperties() const override;
    void setExtraProperty(const QString &key, const QVariant &val) override;

private:
    bool reflection() const;
    bool hasMethod(const QString &methodName) const;

public:
    QString reflectionObjName;
    mutable QObject *reflectionObj { nullptr };
    QString defaultName;
    QIcon defualtIcon;
};

}   // end namespace dfmplugin_computer

#endif   // COMMONENTRYFILEENTITY_H
