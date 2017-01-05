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
