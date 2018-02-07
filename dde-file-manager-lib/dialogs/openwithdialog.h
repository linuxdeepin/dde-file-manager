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

#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include "basedialog.h"
#include "durl.h"

#include <dflowlayout.h>
#include <dlinkbutton.h>

#include <QObject>
#include <QMimeType>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QScrollArea;
class QCheckBox;
QT_END_NAMESPACE

class OpenWithDialogListItem;
class OpenWithDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const DUrl& url, QWidget *parent = 0);
    ~OpenWithDialog();

public slots:
    void openFileByApp();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void initData();
    void checkItem(OpenWithDialogListItem *item);
    void useOtherApplication();
    OpenWithDialogListItem *createItem(const QIcon &icon, const QString &name, const QString &filePath);

    QScrollArea *m_scrollArea = NULL;
    DFlowLayout *m_recommandLayout = NULL;
    DFlowLayout *m_otherLayout = NULL;

    DLinkButton *m_openFileChooseButton = NULL;
    QCheckBox *m_setToDefaultCheckBox = NULL;
    QPushButton *m_cancelButton = NULL;
    QPushButton *m_chooseButton = NULL;
    DUrl m_url;
    QMimeType m_mimeType;

    OpenWithDialogListItem *m_checkedItem = NULL;
};

#endif // OPENWITHDIALOG_H
