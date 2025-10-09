// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIZEGROUPSTRATEGY_H
#define SIZEGROUPSTRATEGY_H

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

namespace dfmplugin_workspace {

/**
 * @brief Size-based grouping strategy implementation
 *
 * This strategy groups files based on their file sizes,
 * organizing them into categories like "Empty", "Small", "Large", etc.
 */
class SizeGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit SizeGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~SizeGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder() const override;
    int getGroupDisplayOrder(const QString &groupKey) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;

private:
    /**
     * @brief Classify file size into a group
     * @param size The file size in bytes
     * @return The corresponding size group key
     */
    QString classifyBySize(qint64 size) const;

    /**
     * @brief Get the size order list
     * @return The size order according to requirements (from smallest to largest)
     */
    static QStringList getSizeOrder();

    /**
     * @brief Get display names for size groups
     * @return Hash map of group keys to display names with size ranges
     */
    static QHash<QString, QString> getDisplayNames();
};

}   // namespace dfmplugin_workspace

#endif   // SIZEGROUPSTRATEGY_H
