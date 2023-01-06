// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNOPTDIALOG_H
#define BURNOPTDIALOG_H
#include "ddialog.h"
#include "durl.h"

#include <QCheckBox>
#include <DLineEdit>
#include <QComboBox>

class BurnOptDialogPrivate;
class BurnOptDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    BurnOptDialog(QString device, QWidget *parent = nullptr);
    ~BurnOptDialog();

    void setISOImage(DUrl image);
    void setJobWindowId(int wid);
    void setDefaultVolName(const QString &volName);
    void setDiscAndFsInfo(int type, QString filesystem, QString version);
    static bool isSupportedUDVersion(const QString &version);
    static bool isSupportedUDMedium(int type);

private:
    QScopedPointer<BurnOptDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BurnOptDialog)

};

#endif // BURNOPTDIALOG_H
