// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWIDTHFILEINDEX_H
#define CHECKBOXWIDTHFILEINDEX_H

#include "indexstatuscheckbox.h"

#include <DDialog>

#include <QDateTime>
#include <QTimer>

namespace dfmplugin_search {

class CheckBoxWidthFileIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWidthFileIndex(QWidget *parent = nullptr);

    void initStatusBar();

protected:
    bool acceptCheckStateChange(Qt::CheckState oldState, Qt::CheckState newState) override;

private:
    struct FileIndexState
    {
        bool querySuccess { false };
        bool enabled { false };
        bool serviceActive { false };
        QString status;
        QString lastUpdateTime;
    };

    struct CommandResult
    {
        bool started { false };
        bool finished { false };
        int exitCode { -1 };
        bool normalExit { false };
        QString standardOutput;
        QString standardError;
    };

    void handleCheckStateChanged(Qt::CheckState state);
    void refreshState();
    FileIndexState queryState() const;
    void applyState(const FileIndexState &state);
    bool enableFileIndex();
    bool disableFileIndex();
    bool restartFileIndex();
    bool confirmDisableFileIndex();
    bool createRefreshIndexFile() const;
    CommandResult runSystemctlCommand(const QStringList &arguments) const;
    QString statusFilePath() const;
    QString refreshFilePath() const;
    QString formatDisplayTime(const QString &isoTime) const;

private:
    QTimer *m_pollTimer { nullptr };
    bool m_syncingState { false };
    bool m_operationInProgress { false };
};

}   // namespace dfmplugin_search

#endif   // CHECKBOXWIDTHFILEINDEX_H
