/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include <QCommandLinkButton>

#include <QObject>
#include <QMimeType>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QScrollArea;
class QCheckBox;
QT_END_NAMESPACE

class OpenWithDialogListItem : public QWidget
{
    Q_OBJECT

public:
    explicit OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    void setChecked(bool checked);
    QString text() const;

protected:
    void resizeEvent(QResizeEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QIcon m_icon;
    DIconButton *m_checkButton;
    QLabel *m_iconLabel;
    QLabel *m_label;
};

class OpenWithDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const QList<DUrl>& urllist, QWidget *parent = nullptr);
    explicit OpenWithDialog(const DUrl &url, QWidget *parent = nullptr);
    ~OpenWithDialog() override;

public slots:
    void openFileByApp();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initUI();
    void initConnect();
    void initData();
    void checkItem(OpenWithDialogListItem *item);
    void useOtherApplication();
    OpenWithDialogListItem *createItem(const QIcon &icon, const QString &name, const QString &filePath);

    QScrollArea *m_scrollArea = nullptr;
    DFlowLayout *m_recommandLayout = nullptr;
    DFlowLayout *m_otherLayout = nullptr;

    QCommandLinkButton *m_openFileChooseButton = nullptr;
    QCheckBox *m_setToDefaultCheckBox = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_chooseButton = nullptr;
    QList<DUrl> m_urllist;
    DUrl m_url;
    QMimeType m_mimeType;

    OpenWithDialogListItem *m_checkedItem = nullptr;
};

#endif // OPENWITHDIALOG_H
