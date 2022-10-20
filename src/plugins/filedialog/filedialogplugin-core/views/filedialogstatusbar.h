/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEDIALOGSTATUSBAR_H
#define FILEDIALOGSTATUSBAR_H

#include "filedialogplugin_core_global.h"

#include <QFrame>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

namespace filedialog_core {

class FileDialogStatusBar : public QFrame
{
    Q_OBJECT
public:
    enum Mode {
        kUnknow,
        kOpen,
        kSave
    };

    explicit FileDialogStatusBar(QWidget *parent = nullptr);

public:
    void setMode(Mode mode);
    void setComBoxItems(const QStringList &list);

    QComboBox *comboBox() const;
    QLineEdit *lineEdit() const;
    QPushButton *acceptButton() const;
    QPushButton *rejectButton() const;

    void addLineEdit(QLabel *label, QLineEdit *edit);
    QString getLineEditValue(const QString &text) const;
    QVariantMap allLineEditsValue() const;
    void addComboBox(QLabel *label, QComboBox *box);
    QString getComboBoxValue(const QString &text) const;
    QVariantMap allComboBoxsValue() const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void changeFileNameEditText(const QString &fileName);

private slots:
    void onWindowTitleChanged(const QString &title);
    void onFileNameTextEdited(const QString &text);

private:
    void initializeUi();
    void initializeConnect();
    void updateLayout();

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Mode curMode { kUnknow };

    QHBoxLayout *contentLayout { nullptr };

    QLabel *titleLabel;
    QLabel *fileNameLabel;
    QLabel *filtersLabel;

    QLineEdit *fileNameEdit;
    QComboBox *filtersComboBox;

    QPushButton *curAcceptButton;
    QPushButton *curRejectButton;

    QList<QPair<QLabel *, QLineEdit *>> customLineEditList;
    QList<QPair<QLabel *, QComboBox *>> customComboBoxList;
};

}

#endif   // FILEDIALOGSTATUSBAR_H
