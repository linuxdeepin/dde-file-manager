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
