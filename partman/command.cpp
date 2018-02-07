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

#include "command.h"

#include <QDebug>
#include <QDir>
#include <QProcess>

namespace PartMan {

bool RunScriptFile(const QString& filepath) {
  QString output, err;
  return RunScriptFile({filepath}, output, err);
}

bool RunScriptFile(const QStringList& args, QString& output, QString& err) {
  Q_ASSERT(!args.isEmpty());
  if (args.isEmpty()) {
    qCritical() << "RunScriptFile() arg is empty!";
    return false;
  }

  // Change working directory.
  const QString current_dir(QFileInfo(args.at(0)).absolutePath());
  if (!QDir::setCurrent(current_dir)) {
    qCritical() << "Failed to change working directory:" << current_dir;
    return false;
  }

  // TODO(xushaohua): Remove bash
  return SpawnCmd("/bin/bash", args, output, err);
}

bool SpawnCmd(const QString& cmd, const QStringList& args) {
  QString output;
  QString err;
  return SpawnCmd(cmd, args, output, err);
}

bool SpawnCmd(const QString& cmd, const QStringList& args, QString& output) {
  QString err;
  return SpawnCmd(cmd, args, output, err);
}

bool SpawnCmd(const QString& cmd, const QStringList& args,
              QString& output, QString& err) {
  QProcess process;
  process.setProgram(cmd);
  process.setArguments(args);
  process.start();
  // Wait for process to finish without timeout.
  process.waitForFinished(-1);
  output = process.readAllStandardOutput();
  err = process.readAllStandardError();
  return (process.exitStatus() == QProcess::NormalExit &&
          process.exitCode() == 0);
}

}  // namespace installer
