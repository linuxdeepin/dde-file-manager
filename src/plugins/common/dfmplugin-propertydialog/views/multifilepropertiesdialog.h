// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIFILEPROPERTIESDIALOG_H
#define MULTIFILEPROPERTIESDIALOG_H

#include "dfmplugin_propertydialog_global.h"
#include "multifilebasicinfowidget.h"
#include "multifilepermissionwidget.h"

#include <DDialog>
#include <DSuggestButton>

#include <QObject>
#include <QScrollArea>
#include <QPushButton>

namespace dfmplugin_propertydialog {

class MultiFilePropertiesDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit MultiFilePropertiesDialog(const QList<QUrl> &urls,
                                       QWidget *parent = Q_NULLPTR);

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void processHeight();
    void saveBtnClicked();
    void handleHideBoxStateChanged(int state);
    void handleOwnerBoxStateChanged(int index);
    void handleGroupBoxStateChanged(int index);
    void handleOtherBoxStateChanged(int index);

private:
    void initUI();
    void initConnect();
    void connectHeightChange(DArrowLineDrawer *w);
    bool changeFilesHideState(int state, QString &strChange);
    bool changeFilesOwnerState(int index, QString &strChange);
    bool changeFilesGroupState(int index, QString &strChange);
    bool changeFilesOtherState(int index, QString &strChange);

    QList<QUrl> fileUrls;
    QFrame *frameHeader { Q_NULLPTR };
    QFrame *scrollWidget { Q_NULLPTR };
    MultiFileBasicInfoWidget *basicInfoWidget { Q_NULLPTR };
    MultiFilePermissionWidget *permInfoWidget { Q_NULLPTR };
    QPushButton *cancelBtn { Q_NULLPTR };
    DSuggestButton *saveBtn { Q_NULLPTR };
    FilePropertyState orgFilesPropertyState;
    FilePropertyState curFilesPropertyState;
};

}

#endif
