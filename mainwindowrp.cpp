#include "mainwindowrp.h"
#include "ui_mainwindowrp.h"


MainWindowRP::MainWindowRP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowRP)
{
    ui->setupUi(this);
    userAgent = new UserAgentRP(this);
}

MainWindowRP::~MainWindowRP()
{
    delete ui;
}

void MainWindowRP::on_pushButton_closeApp_clicked()
{
    QApplication::quit();
}

void MainWindowRP::on_pushButton_doLogin_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    emit attemptLogin(username,password);
}
