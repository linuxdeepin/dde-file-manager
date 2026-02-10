// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGSTATUSBAR_H
#define FILEDIALOGSTATUSBAR_H

#include "filedialogplugin_core_global.h"

#include <dtkwidget_global.h>
#include <DPushButton>

#include <QFrame>
#include <QPushButton>

DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
class DLabel;
class DComboBox;
class DSuggestButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

namespace filedialog_core {
class FileDialog;

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

    DTK_WIDGET_NAMESPACE::DComboBox *comboBox() const;
    DTK_WIDGET_NAMESPACE::DLineEdit *lineEdit() const;
    DTK_WIDGET_NAMESPACE::DSuggestButton *acceptButton() const;
    DTK_WIDGET_NAMESPACE::DPushButton *rejectButton() const;

    void addLineEdit(DTK_WIDGET_NAMESPACE::DLabel *label, DTK_WIDGET_NAMESPACE::DLineEdit *edit);
    QString getLineEditValue(const QString &text) const;
    QVariantMap allLineEditsValue() const;
    void addComboBox(DTK_WIDGET_NAMESPACE::DLabel *label, DTK_WIDGET_NAMESPACE::DComboBox *box);
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
    void updateComboxViewWidth();
    void setAppropriateWidgetFocus();

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Mode curMode { kUnknow };

    QHBoxLayout *contentLayout { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *fileNameLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *filtersLabel { nullptr };

    DTK_WIDGET_NAMESPACE::DLineEdit *fileNameEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *filtersComboBox { nullptr };

    DTK_WIDGET_NAMESPACE::DSuggestButton *curAcceptButton { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *curRejectButton { nullptr };

    QList<QPair<DTK_WIDGET_NAMESPACE::DLabel *, DTK_WIDGET_NAMESPACE::DLineEdit *>> customLineEditList;
    QList<QPair<DTK_WIDGET_NAMESPACE::DLabel *, DTK_WIDGET_NAMESPACE::DComboBox *>> customComboBoxList;
    FileDialog *mainWindow { nullptr };
};

}

#endif   // FILEDIALOGSTATUSBAR_H
