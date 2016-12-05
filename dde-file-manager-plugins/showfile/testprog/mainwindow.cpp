#include "mainwindow.h"
#include "ui_testprog.h"

MainWindow::MainWindow(QString &eventname,QString &filepath,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->event->setText( eventname );
    ui->filename->setText( filepath );
}

MainWindow::~MainWindow()
{
    delete ui;
}
