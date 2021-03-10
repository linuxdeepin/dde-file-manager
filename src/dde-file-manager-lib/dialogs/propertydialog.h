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

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "basedialog.h"
#include "dabstractfileinfo.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dfmevent.h"
#include <views/dfmactionbutton.h>

#include <dplatformwindowhandle.h>

#include <QLabel>
#include <QCloseEvent>
#include <QTextEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QStorageInfo>
#include <QScrollArea>
#include <QVariantAnimation>

#define EXTEND_FRAME_MAXHEIGHT 160

QT_BEGIN_NAMESPACE
class QFrame;
class QCheckBox;
class QTimer;
class QListWidget;
class QButtonGroup;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DDrawer;
class DIconButton;
DWIDGET_END_NAMESPACE

class DAbstractFileInfo;
class UDiskDeviceInfo;
class DUrl;
class ShareInfoFrame;


DFM_BEGIN_NAMESPACE
class DFileStatisticsJob;
DFM_END_NAMESPACE

class DFMRoundBackground : public QObject
{
    Q_OBJECT
public:
    DFMRoundBackground(QWidget *parent, int radius): QObject(parent)
    {
        parent->installEventFilter(this);
        setProperty("radius", radius);
    }
    ~DFMRoundBackground()
    {
        parent()->removeEventFilter(this);
    }

    virtual bool eventFilter(QObject *watched, QEvent *event);
};

DWIDGET_USE_NAMESPACE


class NameTextEdit: public QTextEdit
{
    Q_OBJECT

public:
    explicit NameTextEdit(const QString &text = "", QWidget *parent = nullptr);

    bool isCanceled() const;
    void setIsCanceled(bool isCanceled);

signals:
    void editFinished();

public slots:
    void setPlainText(const QString &text);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool m_isCanceled = false;
};


class GroupTitleLabel: public QLabel
{
    Q_OBJECT

public:
    explicit GroupTitleLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class LinkSectionValueLabel: public SectionValueLabel
{
    Q_OBJECT

public:
    explicit LinkSectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});

    DUrl linkTargetUrl() const;
    void setLinkTargetUrl(const DUrl &linkTargetUrl);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    DUrl m_linkTargetUrl;
};

class PropertyDialog : public DDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(const DFMEvent &event, const DUrl url, QWidget *parent = nullptr);

public:
    void initUI();
    void initConnect();
    void startComputerFolderSize(const DUrl &url);
    void toggleFileExecutable(bool isChecked);
    DUrl getUrl();
    int getFileCount();
    qint64 getFileSize();

    const QList<Dtk::Widget::DDrawer *> &expandGroup() const;
    int contentHeight() const;
    int getDialogHeight() const;

    void loadPluginExpandWidgets();

public slots:
    void raise();
    void updateFolderSize(qint64 size);
    void renameFile();
    void showTextShowFrame();
    void onChildrenRemoved(const DUrl &fileUrl);
    // fileUrl:共享文件夹路径
    void flickFolderToSidebar(const DUrl &fileUrl);
    void onOpenWithBntsChecked(QAbstractButton *w);
    void onHideFileCheckboxChecked(bool checked);
    // 取消共享时停止动画效果
    void onCancelShare();

signals:
    void closed(const DUrl &url);
    void aboutToClosed(const DUrl &url);
    void raised();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void initExpand(QVBoxLayout *layout, DDrawer *expand);

private:
    QFrame *initTagFrame(const DUrl &url);
    void updateInfo(); // when any property has been changed should update the linked properties ASAP, bug 25419
    //属性框的URL需要重定向
    const DUrl getRealUrl();
    bool canChmod(const DAbstractFileInfoPointer &info);

private:
    DFMEvent m_fmevent{};
    DUrl m_url{};
    QString m_absolutePath{};
    qint64 m_size{ 0 };
    bool m_editDisbaled{ false };
    int m_fileCount{ 0 };
    QLabel *m_icon{ nullptr };
    NameTextEdit *m_edit{ nullptr };
    QStackedWidget *m_editStackWidget{ nullptr };
    QFrame *m_textShowFrame{ nullptr };
    DIconButton *m_editButton{ nullptr };
    QCheckBox *m_executableCheckBox{ nullptr };
    SectionValueLabel *m_folderSizeLabel{ nullptr };
    SectionValueLabel *m_containSizeLabel{ nullptr };
    QFrame *m_basicInfoFrame{ nullptr };
    ShareInfoFrame *m_shareinfoFrame{ nullptr };
    QFrame *m_authorityManagementFrame{ nullptr };
    QFrame *m_deviceInfoFrame{ nullptr };
    QListWidget *m_OpenWithListWidget{ nullptr };
    QButtonGroup *m_OpenWithButtonGroup{ nullptr };
    QList<DDrawer *> m_expandGroup;
    DFM_NAMESPACE::DFileStatisticsJob *m_sizeWorker{ nullptr };
    QVBoxLayout *m_mainLayout{ nullptr };
    QFrame *m_wdf{ nullptr };
    QScrollArea *m_scrollArea{ nullptr };
    QFrame      *m_tagInfoFrame{ nullptr };
    QPointer<QVariantAnimation> m_xani {nullptr};
    QPointer<QVariantAnimation> m_gani {nullptr};
    QPointer<QLabel> m_aniLabel {nullptr};


    QList<DDrawer *> addExpandWidget(const QStringList &titleList);

    void initTextShowFrame(const QString &text);
    QFrame *createBasicInfoWidget(const DAbstractFileInfoPointer &info);
    ShareInfoFrame *createShareInfoFrame(const DAbstractFileInfoPointer &info);
    QList<QPair<QString, QString> > createLocalDeviceInfoWidget(const DAbstractFileInfoPointer &info);
    QFrame *createInfoFrame(const QList<QPair<QString, QString> > &properties);
    QListWidget *createOpenWithListWidget(const DAbstractFileInfoPointer &info);
    QFrame *createAuthorityManagementWidget(const DAbstractFileInfoPointer &info);

    QPointer<DPlatformWindowHandle> m_platformWindowHandle {nullptr};
};

#endif // PROPERTYDIALOG_H
