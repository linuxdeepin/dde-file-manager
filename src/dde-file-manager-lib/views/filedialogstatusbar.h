/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef FILEDIALOGSTATUSBAR_H
#define FILEDIALOGSTATUSBAR_H

#include <QFrame>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

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

private:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateLayout();
    void onWindowTitleChanged(const QString &title);

    Mode m_mode = Unknow;

    QHBoxLayout *m_contentLayout;

    QLabel *m_titleLabel;
    QLabel *m_fileNameLabel;
    QLabel *m_filtersLabel;

    QLineEdit *m_fileNameEdit;
    QComboBox *m_filtersComboBox;

    QPushButton *m_acceptButton;
    QPushButton *m_rejectButton;

    QList<QPair<QLabel *, QLineEdit *>> m_customLineEditList;
    QList<QPair<QLabel *, QComboBox *>> m_customComboBoxList;

    friend class DFileDialog;
};

#endif // FILEDIALOGSTATUSBAR_H
