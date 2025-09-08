// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPEGROUPSTRATEGY_H
#define TYPEGROUPSTRATEGY_H

#include "abstractgroupstrategy.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Type-based grouping strategy implementation
 * 
 * This strategy groups files based on their MIME types, organizing them
 * into categories like documents, images, videos, etc.
 */
class TypeGroupStrategy : public AbstractGroupStrategy
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
    QString getGroupKey(const FileItemDataPointer &item) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const override;
    int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileItemDataPointer> &items) const override;
    QString getStrategyName() const override;
    DFMBASE_NAMESPACE::Global::ItemRoles getCorrespondingRole() const override;

private:
    /**
     * @brief Map MIME type to group key
     * @param mimeType The MIME type string
     * @return The corresponding group key
     */
    QString mapMimeTypeToGroup(const QString &mimeType) const;

    /**
     * @brief Type order according to requirements (directories first, then by frequency)
     */
    static const QStringList TYPE_ORDER;

    /**
     * @brief Display names for each type group
     */
    static const QHash<QString, QString> DISPLAY_NAMES;
};

DPWORKSPACE_END_NAMESPACE

#endif // TYPEGROUPSTRATEGY_H 