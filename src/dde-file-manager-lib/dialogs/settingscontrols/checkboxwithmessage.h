// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
    void setText(const QString &text);
    void setMessage(const QString &message);
    QCheckBox *checkBox();

private:
    QWidget *m_widget = nullptr ;
    QCheckBox * m_checkBox = nullptr ;
    QLabel *m_message = nullptr ;
};

#endif // CHECKBOXWITHMESSAGE_H
