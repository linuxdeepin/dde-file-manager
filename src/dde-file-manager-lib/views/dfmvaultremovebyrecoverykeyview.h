// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
