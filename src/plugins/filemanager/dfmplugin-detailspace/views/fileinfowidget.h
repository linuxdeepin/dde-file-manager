// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOWIDGET_H
#define FILEINFOWIDGET_H

#include "dfmplugin_detailspace_global.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QUrl>
#include <QFrame>
#include <QGridLayout>
#include <QMultiMap>

namespace dfmplugin_detailspace {

/**
 * @brief FileInfoWidget displays basic file information fields
 *
 * Design principles:
 * - All 7 KeyValueLabel widgets are created once in constructor
 * - Single QGridLayout created once in constructor
 * - Signal connections established once in constructor
 * - setUrl() only updates data via setRightValue() and setVisible()
 * - No widget deletion/recreation on URL change - prevents UI flicker
 *
 * This replaces FileBaseInfoView which had memory leaks and caused UI flicker
 * due to recreating widgets on every URL change.
 */
class FileInfoWidget : public QFrame
{
    Q_OBJECT
public:
    explicit FileInfoWidget(QWidget *parent = nullptr);
    ~FileInfoWidget() override = default;

    void setUrl(const QUrl &url);

private:
    void initUI();
    void initConnections();

    void resetAllFields();
    void applyFieldFilters(const QUrl &url);
    void applyFieldExpansions(const QUrl &url);
    void fillFieldValues(const QUrl &url);

    QString formatDateTime(const QDateTime &time) const;

private slots:
    void onImageMediaInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void onVideoMediaInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void onAudioMediaInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);

private:
    // Core 7 fields - created once, never deleted
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileName { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileViewSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileDuration { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileAccessTime { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *m_fileModifyTime { nullptr };

    // Layout - created once
    QGridLayout *m_gridLayout { nullptr };

    // Field mapping for extension support
    QMultiMap<BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *> m_fieldMap;

    // Dynamic expansion widgets (these may be created/deleted for each URL)
    QList<DFMBASE_NAMESPACE::KeyValueLabel *> m_dynamicFields;

    QUrl m_currentUrl;
};

}   // namespace dfmplugin_detailspace

#endif   // FILEINFOWIDGET_H
