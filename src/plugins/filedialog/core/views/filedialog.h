// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "filedialogplugin_core_global.h"

#include <dfm-base/widgets/filemanagerwindow.h>

#include <QWidget>
#include <QFileDialog>

namespace filedialog_core {

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
    QUrl lastVisitedUrl() const;

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

    void urlSchemeEnable(const QString &scheme, bool enable);
    void setCurrentInputName(const QString &name);
    void addCustomWidget(CustomWidgetType type, const QString &data);
    QVariant getCustomWidgetValue(CustomWidgetType type, const QString &text) const;
    QVariantMap allCustomWidgetsValue(CustomWidgetType type) const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void setHideOnAccept(bool enable);
    bool hideOnAccept() const;

    QUrl getcurrenturl() const;
    bool checkFileSuffix(const QString &filename, QString &suffix);
    FileDialogStatusBar *statusBar() const;

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void selectedNameFilterChanged();
    void initialized();

public Q_SLOTS:
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();
    void onAcceptButtonClicked();

private Q_SLOTS:
    void onRejectButtonClicked();
    void onCurrentInputNameChanged();
    void updateAcceptButtonState();
    void handleEnterPressed();
    void handleUrlChanged(const QUrl &url);
    void onViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected);
    void onViewItemClicked(const QVariantMap &data);
    void handleRenameStartAcceptBtn(const quint64 windowID, const QUrl &url);
    void handleRenameEndAcceptBtn(const quint64 windowID, const QUrl &url);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void initEventsConnect();
    void updateViewState();
    void adjustPosition(QWidget *w);
    QString modelCurrentNameFilter() const;
    
    bool isFileNameEditFocused() const;
    void handleEnterInSaveMode();
    void handleEnterInOpenMode();

private:
    QScopedPointer<FileDialogPrivate> d;
    QStringList curNameFilters {};
};

}

#endif   // FILEDIALOG_H
