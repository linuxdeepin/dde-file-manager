#ifndef FILECONFLICTDIALOG_H
#define FILECONFLICTDIALOG_H

#include "dbasedialog.h"


class QHBoxLayout;
class FileItemInfo;

class FileConflictDialog : public DBaseDialog
{
    Q_OBJECT
public:
    explicit FileConflictDialog(QWidget *parent = 0);
    ~FileConflictDialog();

    QHBoxLayout* createFileLayout(QString icon, QString size, QString time, QString fileTip);

signals:

public slots:
    void addConflictFiles(QString newFile, QString oldFile);
};

#endif // FILECONFLICTDIALOG_H
