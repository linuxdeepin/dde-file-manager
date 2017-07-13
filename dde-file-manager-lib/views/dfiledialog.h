#ifndef DFILEDIALOG_H
#define DFILEDIALOG_H

#include "dfilemanagerwindow.h"
#include "dfileview.h"
#include "dfmabstracteventhandler.h"

#include <QFileDialog>

DFM_USE_NAMESPACE

class DFileDialogPrivate;
class FileDialogStatusBar;
class DFileDialog : public DFileManagerWindow
{
    Q_OBJECT

public:
    explicit DFileDialog(QWidget *parent = 0);
    ~DFileDialog();

    void setDirectory(const QString & directory);
    void setDirectory(const QDir & directory);
    QDir directory() const;

    void setDirectoryUrl(const DUrl & directory);
    QUrl directoryUrl() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString modelCurrentNameFilter() const;
    QString selectedNameFilter() const;
    void selectNameFilterByIndex(int index);
    int selectedNameFilterIndex() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setViewMode(DFileView::ViewMode mode);
    DFileView::ViewMode viewMode() const;

    void setFileMode(QFileDialog::FileMode mode);

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    bool testOption(QFileDialog::Option option) const;
    QFileDialog::Options options() const;

    void setCurrentInputName(const QString &name);

    DFileView *getFileView() const;

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

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void adjustPosition(QWidget *w);

    bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target = 0, QVariant *resultData = 0) Q_DECL_OVERRIDE;

private:
    void handleNewView(DFMBaseView *view) Q_DECL_OVERRIDE;
    FileDialogStatusBar *statusBar() const;

    void onAcceptButtonClicked();
    void onRejectButtonClicked();
    void onCurrentInputNameChanged();
    void handleEnterPressed();

    QScopedPointer<DFileDialogPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileDialog)
};

#endif // DFILEDIALOG_H
