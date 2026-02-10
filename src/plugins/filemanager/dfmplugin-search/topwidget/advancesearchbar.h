// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCESEARCHBAR_H
#define ADVANCESEARCHBAR_H

#include "dfmplugin_search_global.h"

#include <dboxwidget.h>

#include <QScrollArea>

namespace dfmplugin_search {

class AdvanceSearchBarPrivate;
class AdvanceSearchBar : public QScrollArea
{
    Q_OBJECT
public:
    explicit AdvanceSearchBar(QWidget *parent = nullptr);
    void resetForm();
    void refreshOptions(const QUrl &url);

public slots:
    void onOptionChanged();
    void onResetButtonPressed();

protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    void initUiForSizeMode();

private:
    AdvanceSearchBarPrivate *d = nullptr;
};

}

#endif   // ADVANCESEARCHBAR_H
