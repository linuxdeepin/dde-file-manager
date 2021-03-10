/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#ifndef DFMVAULTREMOVEBYRECOVERYKEYVIEW_H
#define DFMVAULTREMOVEBYRECOVERYKEYVIEW_H

#include <QWidget>
#include <QScopedPointer>

class QPlainTextEdit;

class DFMVaultRemoveByRecoverykeyViewPrivate;
class DFMVaultRemoveByRecoverykeyView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultRemoveByRecoverykeyView(QWidget *parent = nullptr);
    ~DFMVaultRemoveByRecoverykeyView() override;

    /**
    * @brief    获取凭证
    */
    QString getRecoverykey();

    /**
    * @brief    清空凭证
    */
    void clear();

    void showAlertMessage(const QString &text, int duration = 3000);
public slots:
    void onRecoveryKeyChanged();

private:
    // 输入凭证后，对凭证添加“-”
    int afterRecoveryKeyChanged(QString &str);

    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QPlainTextEdit *m_keyEdit {nullptr};

    QScopedPointer<DFMVaultRemoveByRecoverykeyViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMVaultRemoveByRecoverykeyView)
};

#endif // DFMVAULTREMOVEBYRECOVERYKEYVIEW_H
