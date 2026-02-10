// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIDDENFILEFILTER_H
#define HIDDENFILEFILTER_H

#include "models/modeldatahandler.h"

namespace ddplugin_organizer {

class HiddenFileFilter : public QObject, public ModelDataHandler
{
    Q_OBJECT
public:
    explicit HiddenFileFilter();
    ~HiddenFileFilter();
    inline bool showHiddenFiles() const {return show;}
    void refreshModel();
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
    bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {}) override;
protected slots:
    void updateFlag();
    void hiddenFlagChanged(bool showHidden);
protected:
    bool show = false;
};

}
#endif // HIDDENFILEFILTER_H
