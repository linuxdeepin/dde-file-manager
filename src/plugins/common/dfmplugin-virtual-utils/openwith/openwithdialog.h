/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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

#include "dfmplugin_utils_global.h"

#include "dialogs/basedialog/basedialog.h"

#include <dflowlayout.h>
#include <DIconButton>

#include <QCommandLinkButton>
#include <QUrl>
#include <QMimeType>

QT_BEGIN_NAMESPACE
class QPushButton;
class QScrollArea;
class QCheckBox;
QT_END_NAMESPACE

namespace dfmplugin_utils {

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
    QIcon icon;
    DTK_WIDGET_NAMESPACE::DIconButton *checkButton;
    QLabel *iconLabel;
    QLabel *label;
};

class OpenWithDialog : public DFMBASE_NAMESPACE::BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const QList<QUrl> &list, QWidget *parent = nullptr);
    explicit OpenWithDialog(const QUrl &url, QWidget *parent = nullptr);
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

    QScrollArea *scrollArea { nullptr };
    DTK_WIDGET_NAMESPACE::DFlowLayout *recommandLayout { nullptr };
    DTK_WIDGET_NAMESPACE::DFlowLayout *otherLayout { nullptr };

    QCommandLinkButton *openFileChooseButton { nullptr };
    QCheckBox *setToDefaultCheckBox { nullptr };
    QPushButton *cancelButton { nullptr };
    QPushButton *chooseButton { nullptr };
    QList<QUrl> urlList;
    QUrl curUrl;
    QMimeType mimeType;

    OpenWithDialogListItem *checkedItem { nullptr };
};
}
#endif   // OPENWITHDIALOG_H
