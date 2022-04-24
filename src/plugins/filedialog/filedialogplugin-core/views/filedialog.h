/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "filedialogplugin_core_global.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QWidget>
#include <QFileDialog>

DIALOGCORE_BEGIN_NAMESPACE

class FileDialogPrivate;
class FileDialogStatusBar;
class FileDialog : public DFMBASE_NAMESPACE::FileManagerWindow
{
    Q_OBJECT
    friend class FileDialogPrivate;

public:
    enum CustomWidgetType {
        kLineEditType = 0,
        kComboBoxType = 1
    };

    explicit FileDialog(const QUrl &url, QWidget *parent = nullptr);
    virtual ~FileDialog() override;

    void cd(const QUrl &url) override;
    bool saveClosedSate() const override;
    void updateAsDefaultSize();

public:
    QFileDialog::ViewMode currentViewMode() const;

    void setDirectory(const QString &directory);
    void setDirectory(const QDir &directory);
    QDir directory() const;

    void setDirectoryUrl(const QUrl &directory);
    QUrl directoryUrl() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;

    void selectNameFilter(const QString &filter);
    QString selectedNameFilter() const;

    void selectNameFilterByIndex(int index);
    int selectedNameFilterIndex() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setFileMode(QFileDialog::FileMode mode);
    void setAllowMixedSelection(bool on);

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    bool testOption(QFileDialog::Option option) const;
    QFileDialog::Options options() const;

    void addDisableUrlScheme(const QString &scheme);
    void setCurrentInputName(const QString &name);
    void addCustomWidget(CustomWidgetType type, const QString &data);
    QVariant getCustomWidgetValue(CustomWidgetType type, const QString &text) const;
    QVariantMap allCustomWidgetsValue(CustomWidgetType type) const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void setHideOnAccept(bool enable);
    bool hideOnAccept() const;

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void currentUrlChanged();
    void selectedNameFilterChanged();

public Q_SLOTS:
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();

private Q_SLOTS:
    void onAcceptButtonClicked();
    void onRejectButtonClicked();
    void onCurrentInputNameChanged();
    void updateAcceptButtonState();
    void handleEnterPressed();
    void handleUrlChanged(const QUrl &url);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void updateViewState(const QString &scheme);
    FileDialogStatusBar *statusBar() const;
    void adjustPosition(QWidget *w);
    void installDFMEventFilter();
    QString modelCurrentNameFilter() const;

private:
    QScopedPointer<FileDialogPrivate> d;
};

DIALOGCORE_END_NAMESPACE

#endif   // FILEDIALOG_H
