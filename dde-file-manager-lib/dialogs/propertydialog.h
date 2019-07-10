/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QLabel>
#include <QCloseEvent>
#include <QTextEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QStorageInfo>
#define EXTEND_FRAME_MAXHEIGHT 160

QT_BEGIN_NAMESPACE
class QFrame;
class QCheckBox;
class QTimer;
class QListWidget;
class QButtonGroup;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DExpandGroup;
DWIDGET_END_NAMESPACE

class DAbstractFileInfo;
class UDiskDeviceInfo;
class DUrl;
class ShareInfoFrame;

DFM_BEGIN_NAMESPACE
class DFileStatisticsJob;
DFM_END_NAMESPACE

DWIDGET_USE_NAMESPACE


class NameTextEdit: public QTextEdit
{
    Q_OBJECT

public:
    explicit NameTextEdit(const QString &text="", QWidget *parent=0);

    bool isCanceled() const;
    void setIsCanceled(bool isCanceled);

signals:
    void editFinished();

public slots:
    void setPlainText(const QString & text);

protected:
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

private:
    bool m_isCanceled = false;
};


class GroupTitleLabel: public QLabel
{
    Q_OBJECT

public:
    explicit GroupTitleLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class LinkSectionValueLabel: public SectionValueLabel
{
    Q_OBJECT

public:
    explicit LinkSectionValueLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});

    DUrl linkTargetUrl() const;
    void setLinkTargetUrl(const DUrl &linkTargetUrl);

protected:
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    DUrl m_linkTargetUrl;
};

class PropertyDialog : public DDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(const DFMEvent &event, const DUrl url, QWidget *parent = 0);

public:
    void initUI();
    void initConnect();
    void startComputerFolderSize(const DUrl &url);
    void toggleFileExecutable(bool isChecked);
    DUrl getUrl();
    int getFileCount();
    qint64 getFileSize();

    DExpandGroup *expandGroup() const;
    int contentHeight() const;

    void loadPluginExpandWidgets();

public slots:
    void raise();
    void updateFolderSize(qint64 size);
    void renameFile();
    void showTextShowFrame();
    void onChildrenRemoved(const DUrl &fileUrl);
    void flickFolderToSidebar();
    void onOpenWithBntsChecked(QAbstractButton *w);
    void onExpandChanged(const bool& e);

signals:
    void closed(const DUrl& url);
    void aboutToClosed(const DUrl& url);
    void raised();

protected:
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

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
    QFrame* m_textShowFrame{ nullptr };
    QPushButton* m_editButton{ nullptr };
    QCheckBox * m_executableCheckBox{ nullptr };
    SectionValueLabel* m_folderSizeLabel{ nullptr };
    SectionValueLabel* m_containSizeLabel{ nullptr };
    QFrame *m_basicInfoFrame{ nullptr };
    ShareInfoFrame* m_shareinfoFrame{ nullptr };
    QFrame* m_authorityManagementFrame{ nullptr };
    QFrame *m_localDeviceInfoFrame{ nullptr };
    QFrame *m_deviceInfoFrame{ nullptr };
    QFrame *m_OpenWithFrame{ nullptr };
    QListWidget* m_OpenWithListWidget{ nullptr };
    QButtonGroup* m_OpenWithButtonGroup{ nullptr };
    DExpandGroup* m_expandGroup{ nullptr };
    DFM_NAMESPACE::DFileStatisticsJob* m_sizeWorker{ nullptr };
    QVBoxLayout* m_mainLayout{ nullptr };
    QWidget* m_wdf{ nullptr };

    DExpandGroup *addExpandWidget(const QStringList &titleList);

    void initTextShowFrame(const QString& text);
    QFrame *createBasicInfoWidget(const DAbstractFileInfoPointer &info);
    ShareInfoFrame* createShareInfoFrame(const DAbstractFileInfoPointer &info);
    QList<QPair<QString, QString> > createLocalDeviceInfoWidget(const QStorageInfo &info);
    QList<QPair<QString, QString> > createLocalDeviceInfoWidget(const UDiskDeviceInfoPointer &info);
    QFrame *createInfoFrame(const QList<QPair<QString, QString> > &properties);
    QListWidget *createOpenWithListWidget(const DAbstractFileInfoPointer &info);
    QFrame *createAuthorityManagementWidget(const DAbstractFileInfoPointer &info);
};

#endif // PROPERTYDIALOG_H
