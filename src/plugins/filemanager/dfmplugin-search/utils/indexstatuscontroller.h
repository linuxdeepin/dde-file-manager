// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXSTATUSCONTROLLER_H
#define INDEXSTATUSCONTROLLER_H

#include "abstractindexclient.h"
#include "indexstatuscheckbox.h"

#include <functional>

namespace dfmplugin_search {

struct IndexStatusControllerOptions
{
    QString logTag;
    QString inactiveText;
    QString indexingInitialText;
    QString indexingFilesText;
    QString indexingItemsText;
    QString failedMainText;
    QString failedLinkText;
    QString completedMainText;
    QString completedLinkText;
    std::function<bool(const QString &, AbstractIndexClient::TaskType)> shouldHandleEvent;
};

class IndexStatusController : public QObject
{
    Q_OBJECT

public:
    enum class CheckContext {
        None,
        ResetIndex,
        InitStatus
    };

    IndexStatusController(IndexStatusCheckBox *view,
                          AbstractIndexClient *client,
                          const IndexStatusControllerOptions &options,
                          QObject *parent = nullptr);

    void syncCheckedState(bool enabled);
    void connectToBackend();
    void initStatusBar();

private:
    bool shouldHandleIndexEvent(const QString &path, AbstractIndexClient::TaskType type) const;

private:
    IndexStatusCheckBox *m_view { nullptr };
    AbstractIndexClient *m_client { nullptr };
    IndexStatusControllerOptions m_options;
    CheckContext m_checkContext { CheckContext::None };
};

}   // namespace dfmplugin_search

#endif   // INDEXSTATUSCONTROLLER_H
