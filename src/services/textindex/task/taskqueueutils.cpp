// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskqueueutils.h"

#include "utils/indexutility.h"

#include <QQueue>

SERVICETEXTINDEX_USE_NAMESPACE

namespace {

QString rewritePathForDirectoryMove(const QString &path,
                                    const QString &normalizedFromPath,
                                    const QString &toPath)
{
    if (path.isEmpty()) {
        return path;
    }

    return PathCalculator::calculateNewPathForDirectoryMove(path, normalizedFromPath, toPath);
}

QStringList rewritePathListForDirectoryMove(const QStringList &paths,
                                            const QString &normalizedFromPath,
                                            const QString &toPath,
                                            bool *changed = nullptr)
{
    QStringList rewrittenPaths;
    rewrittenPaths.reserve(paths.size());

    bool anyChanged = false;
    for (const QString &path : paths) {
        const QString rewrittenPath = rewritePathForDirectoryMove(path, normalizedFromPath, toPath);
        rewrittenPaths.append(rewrittenPath);
        anyChanged = anyChanged || (rewrittenPath != path);
    }

    if (changed) {
        *changed = anyChanged;
    }

    return rewrittenPaths;
}

QHash<QString, QString> rewriteMoveMapForDirectoryMove(const QHash<QString, QString> &movedFiles,
                                                       const QString &normalizedFromPath,
                                                       const QString &toPath,
                                                       bool *changed = nullptr)
{
    QHash<QString, QString> rewrittenMoves;
    bool anyChanged = false;

    for (auto it = movedFiles.constBegin(); it != movedFiles.constEnd(); ++it) {
        const QString rewrittenFrom = rewritePathForDirectoryMove(it.key(), normalizedFromPath, toPath);
        const QString rewrittenTo = rewritePathForDirectoryMove(it.value(), normalizedFromPath, toPath);
        rewrittenMoves.insert(rewrittenFrom, rewrittenTo);
        anyChanged = anyChanged || rewrittenFrom != it.key() || rewrittenTo != it.value();
    }

    if (changed) {
        *changed = anyChanged;
    }

    return rewrittenMoves;
}

}   // namespace

QList<TaskQueueUtils::DirectoryMovePlan> TaskQueueUtils::buildDirectoryMovePlans(const QHash<QString, QString> &movedFiles)
{
    QList<DirectoryMovePlan> plans;
    plans.reserve(movedFiles.size());

    for (auto it = movedFiles.constBegin(); it != movedFiles.constEnd(); ++it) {
        if (!PathCalculator::isDirectoryMove(it.value())) {
            continue;
        }

        DirectoryMovePlan plan;
        plan.fromPath = it.key();
        plan.toPath = it.value();
        plans.append(plan);
    }

    return plans;
}

bool TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(QQueue<TaskQueueItem> &taskQueue,
                                                        const QString &fromPath,
                                                        const QString &toPath)
{
    if (taskQueue.isEmpty() || fromPath.isEmpty() || toPath.isEmpty()) {
        return false;
    }

    const QString normalizedFromPath = PathCalculator::normalizeDirectoryPath(fromPath);
    bool queueChanged = false;

    // Fix: queued incremental tasks may still carry stale paths after a directory rename/move.
    // Rewrite them before execution so TaskManager stays a scheduler while this helper owns
    // the pure path migration logic.
    for (TaskQueueItem &item : taskQueue) {
        bool itemChanged = false;

        const QString rewrittenPath = rewritePathForDirectoryMove(item.path, normalizedFromPath, toPath);
        itemChanged = itemChanged || (rewrittenPath != item.path);
        item.path = rewrittenPath;

        bool pathListChanged = false;
        item.pathList = rewritePathListForDirectoryMove(item.pathList, normalizedFromPath, toPath, &pathListChanged);
        itemChanged = itemChanged || pathListChanged;

        bool fileListChanged = false;
        item.fileList = rewritePathListForDirectoryMove(item.fileList, normalizedFromPath, toPath, &fileListChanged);
        itemChanged = itemChanged || fileListChanged;

        bool moveMapChanged = false;
        item.movedFiles = rewriteMoveMapForDirectoryMove(item.movedFiles, normalizedFromPath, toPath, &moveMapChanged);
        itemChanged = itemChanged || moveMapChanged;

        queueChanged = queueChanged || itemChanged;
    }

    return queueChanged;
}
