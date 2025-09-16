// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPEGROUPSTRATEGY_H
#define TYPEGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Type-based grouping strategy implementation
 *
 * This strategy groups files based on their MIME types, organizing them
 * into categories like documents, images, videos, etc.
 */
class TypeGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit TypeGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TypeGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder() const override;
    int getGroupDisplayOrder(const QString &groupKey) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;

private:
    /**
     * @brief Map MIME type to group key
     * @param mimeType The MIME type string
     * @return The corresponding group key
     */
    QString mapMimeTypeToGroup(const QString &mimeType) const;

    /**
     * @brief Get the type order list
     * @return The type order according to requirements (directories first, then by frequency)
     */
    static QStringList getTypeOrder();

    /**
     * @brief Get display names for type groups
     * @return Hash map of group keys to display names for each type group
     */
    static QHash<QString, QString> getDisplayNames();
};

DPWORKSPACE_END_NAMESPACE

#endif   // TYPEGROUPSTRATEGY_H