/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#include "structs.h"

namespace PartMan {

QDebug& operator<<(QDebug& debug, const OsType& os_type) {
  QString os;
  switch (os_type) {
    case OsType::Linux: {
      os = "Linux";
      break;
    }
    case OsType::Windows: {
      os = "Windows";
      break;
    }
    case OsType::Mac: {
      os = "MacOs";
      break;
    }
    default: {
      // pass
    }
  }
  debug << os;
  return debug;
}

QDebug& operator<<(QDebug& debug, const PartitionTableType& table_type) {
  QString table;
  switch (table_type) {
    case PartitionTableType::GPT: {
      table = "GPT";
      break;
    }
    case PartitionTableType::MsDos: {
      table = "MsDos";
      break;
    }
    case PartitionTableType::Others: {
      table = "Others";
      break;
    }
    case PartitionTableType::Unknown: {
      table = "Unknown";
      break;
    }
    default: {
      // pass
    }
  }
  debug << table;
  return debug;
}

}  // namespace installer
