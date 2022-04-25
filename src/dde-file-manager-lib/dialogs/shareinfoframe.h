/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef SHAREINFOFRAME_H
#define SHAREINFOFRAME_H

#include <QFrame>
#include <QTimer>
#include <QPointer>

#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QLineEdit;
class QTextBrowser;
class QPushButton;
QT_END_NAMESPACE

class ShareInfoFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent = 0);
    ~ShareInfoFrame();

    void initUI();
    void initConnect();
    void setFileinfo(const DAbstractFileInfoPointer &fileinfo);
    bool checkShareName();

signals:
    void folderShared(const QString &filePath);
    void unfolderShared();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
public slots:
    void handleCheckBoxChanged(const bool &checked);
    void handleShareNameFinished();
    // 屏蔽NameChanged信号
    void handleShareNameChanged(const QString &str);
    void handlePermissionComboxChanged(const int &index);
    void handleAnonymityComboxChanged(const int &index);
    void handShareInfoChanged();
    bool doShareInfoSetting();
    void updateShareInfo(const QString &filePath);
    void activateWidgets();
    void disactivateWidgets();
private slots:
    void setOrModifySharePassword();

private:
    DAbstractFileInfoPointer m_fileinfo;
    QCheckBox *m_shareCheckBox = nullptr;
    QLineEdit *m_shareNamelineEdit = nullptr;
    QComboBox *m_permissoComBox = nullptr;
    QComboBox *m_anonymityCombox = nullptr;
    QLabel *m_netScheme = nullptr;
    QLineEdit *m_networkAddrLabel = nullptr;
    QLineEdit *m_userNamelineEdit = nullptr;
    QLineEdit *m_sharePasswordlineEdit = nullptr;
    QPointer<QWidget> m_sharePropertyBkgWidget = nullptr;
    QTextBrowser *m_shareNotes = nullptr;
    QPushButton *splitLineGray = nullptr;
    bool m_isSharePasswordSet = false;
    QTimer *m_updateIp = nullptr;
    //QTimer *m_jobTimer;
    QString m_selfIp;
};

#endif // SHAREINFOFRAME_H
