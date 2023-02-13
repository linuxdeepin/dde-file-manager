// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
