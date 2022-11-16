/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BUILTININTERFACE_H
#define BUILTININTERFACE_H

#include <QFile>
#include <QStandardPaths>

namespace dfm_upgrade {

inline constexpr char kUpgradeFlag[] = "dfm-upgraded.lock";
inline constexpr char kArgDesktop[] = "Desktop";
inline constexpr char kArgFileManger[] = "FileManager";

inline QString upgradeConfigDir() {
    return QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager";
}

inline bool isNeedUpgrade() {
    return QFile::exists(upgradeConfigDir() + "/" + kUpgradeFlag);
}

inline bool tryUpgrade(const QMap<QString , QString> &args) {
    // load library
    return true;
}

}

#endif // BUILTININTERFACE_H
