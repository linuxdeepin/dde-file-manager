// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGSTATUSBAR_H
#define FILEDIALOGSTATUSBAR_H

#include <dtkwidget_global.h>

#include <QFrame>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DSuggestButton;
DWIDGET_END_NAMESPACE

class FileDialogStatusBar : public QFrame
{
    Q_OBJECT
public:
    enum Mode {
        Unknow,
        Open,
        Save
    };

    explicit FileDialogStatusBar(QWidget *parent = 0);

    void setMode(Mode mode);

    void setComBoxItems(const QStringList &list);
    QComboBox *comboBox() const;

    QLineEdit *lineEdit() const;

    Dtk::Widget::DSuggestButton *acceptButton() const;
    QPushButton *rejectButton() const;

    void addLineEdit(QLabel *label, QLineEdit *edit);
    QString getLineEditValue(const QString &text) const;
    QVariantMap allLineEditsValue() const;
    void addComboBox(QLabel *label, QComboBox *box);
    QString getComboBoxValue(const QString &text) const;
    QVariantMap allComboBoxsValue() const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void changeFileNameWithoutSuffix(const QString &fileName);

private:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateLayout();
    void onWindowTitleChanged(const QString &title);
    void updateComboxWidth();

    Mode m_mode = Unknow;

    QHBoxLayout *m_contentLayout;

    QLabel *m_titleLabel;
    QLabel *m_fileNameLabel;
    QLabel *m_filtersLabel;

    QLineEdit *m_fileNameEdit;
    QComboBox *m_filtersComboBox;

    DTK_WIDGET_NAMESPACE::DSuggestButton *m_acceptButton;
    QPushButton *m_rejectButton;

    QList<QPair<QLabel *, QLineEdit *>> m_customLineEditList;
    QList<QPair<QLabel *, QComboBox *>> m_customComboBoxList;

    friend class DFileDialog;
};

#endif // FILEDIALOGSTATUSBAR_H
