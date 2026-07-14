// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIFILEPERMISSIONWIDGET_H
#define MULTIFILEPERMISSIONWIDGET_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <DArrowLineDrawer>
#include <DComboBox>

namespace dfmplugin_propertydialog {

class MultiFilePermissionWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit MultiFilePermissionWidget(const QList<QUrl> &urls,
                                       QWidget *parent = Q_NULLPTR);

    void getOrgPermissonBoxState(FilePropertyState &states);
    void disablePermissionComboBox();

Q_SIGNALS:
    void ownerComboxChanged(int index);
    void groupComboxChanged(int index);
    void otherComboxChanged(int index);

private Q_SLOTS:
    void onOwnerComboBoxChanged(int index);
    void onGroupComboBoxChanged(int index);
    void onOtherComboBoxChanged(int index);

private:
    void initUI();
    void loadData(const QList<QUrl> &urls);
    void initConnect();
    bool canChmodByFs(const QUrl &url);
    bool canChmodByFile(const FileInfoPointer &info);
    void updateComboBoxViewPalette();

    DTK_WIDGET_NAMESPACE::DComboBox *ownerComboBox { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DComboBox *groupComboBox { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DComboBox *otherComboBox { Q_NULLPTR };
};

}

#endif
