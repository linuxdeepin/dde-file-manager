// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCKENTRYFILEENTITY_H
#define BLOCKENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmplugin_computer {

namespace WinVolTagKeys {
inline constexpr char kWinUUID[] { "winUUID" };
inline constexpr char kWinLabel[] { "winLabel" };
inline constexpr char kWinDrive[] { "winDrive" };
}   // namespace WinVolTagKeys

class BlockEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT
public:
    explicit BlockEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order() const override;

    virtual quint64 sizeTotal() const override;
    virtual quint64 sizeUsage() const override;
    virtual void refresh() override;
    virtual QUrl targetUrl() const override;
    virtual bool isAccessable() const override;
    virtual bool renamable() const override;

private:
    QVariant getProperty(const char *const key) const;
    bool showSizeAndProgress() const;
    QUrl mountPoint() const;
    void loadDiskInfo();
    void loadWindowsVoltag();
    void resetWindowsVolTag();
    bool isSiblingOfRoot() const;
};

}

#endif   // BLOCKENTRYFILEENTITY_H
