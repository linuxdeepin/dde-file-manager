// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWITHMESSAGE_H
#define CHECKBOXWITHMESSAGE_H

#include <QWidget>

class QCheckBox;
class QLabel;
class CheckBoxWithMessage : public QWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWithMessage(QWidget *parent = nullptr);
    void setDisplayText(const QString &checkText, const QString &msg);
    void setChecked(bool checked);

Q_SIGNALS:
    void stateChanged(int);

private:
    QCheckBox *checkBox = nullptr;
    QLabel *msgLabel = nullptr;
};

#endif   // CHECKBOXWITHMESSAGE_H
