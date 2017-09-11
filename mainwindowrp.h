#ifndef MAINWINDOWRP_H
#define MAINWINDOWRP_H

#include <QMainWindow>
#include "useragentrp.h"
namespace Ui {
class MainWindowRP;
}

class MainWindowRP : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowRP(QWidget *parent = 0);
    ~MainWindowRP();

private:
    Ui::MainWindowRP *ui;
    UserAgentRP * userAgent;


signals:
    void attemptLogin(QString username,QString password);

public slots:

private slots:
    void on_pushButton_closeApp_clicked();

    void on_pushButton_doLogin_clicked();


};

#endif // MAINWINDOWRP_H
