// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAMEGROUPSTRATEGY_H
#define NAMEGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

#include <QChar>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Name-based grouping strategy implementation
 *
 * This strategy groups files based on the first character of their names,
 * supporting both English letters and Chinese characters with pinyin conversion.
 */
class NameGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit NameGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~NameGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const override;
    int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;

private:
    /**
     * @brief Classify the first character of a name
     * @param ch The first character to classify
     * @return The corresponding group key
     */
    QString classifyFirstCharacter(const QChar &ch) const;

    /**
     * @brief Check if a character is Chinese
     * @param ch The character to check
     * @return True if the character is Chinese, false otherwise
     */
    bool isChinese(const QChar &ch) const;

    /**
     * @brief Get pinyin for a Chinese character
     * @param ch The Chinese character
     * @return The first pinyin reading, empty if conversion fails
     */
    QString getPinyin(const QChar &ch) const;

private:
    /**
     * @brief Get the name order list
     * @return The name order according to requirements
     */
    static QStringList getNameOrder();

    /**
     * @brief Get display names for name groups
     * @return Hash map of group keys to display names
     */
    static QHash<QString, QString> getDisplayNames();
};

DPWORKSPACE_END_NAMESPACE

#endif   // NAMEGROUPSTRATEGY_H
