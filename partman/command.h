// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#ifndef INSTALLER_BASE_COMMAND_H
#define INSTALLER_BASE_COMMAND_H

#include <QStringList>

namespace PartMan {

// Run a script file at |filepath|, no matter |filepath| is executable or not.
// Current working directory is changed to folder of |filepath|.
// Returns true if |filepath| executed and exited with 0.
bool RunScriptFile(const QString& filepath);
// First argument in |args| is the path to script file.
// |output| and |err| are content of stdout and stderr.
bool RunScriptFile(const QStringList& args, QString& output, QString& err);

// Run |cmd| with |args| in background and returns its result.
bool SpawnCmd(const QString& cmd, const QStringList& args);
bool SpawnCmd(const QString& cmd, const QStringList& args, QString& output);
bool SpawnCmd(const QString& cmd, const QStringList& args, QString& output,
              QString& err);

}  // namespace installer

#endif  // INSTALLER_BASE_COMMAND_H
