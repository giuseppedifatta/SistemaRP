#ifndef MAINWINDOWRP_H
#define MAINWINDOWRP_H

#include <QMainWindow>
#include <QMessageBox>
#include "useragentrp.h"
#include "proceduravoto.h"
#include <vector>
namespace Ui {
class MainWindowRP;
}

class MainWindowRP : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowRP(QWidget *parent = 0);
    ~MainWindowRP();

    enum InterfacceRP{
        login,
        procedure,
        scrutinio,
        risultati
    };

private:
    Ui::MainWindowRP *ui;
    UserAgentRP * userAgent;

    int idProceduraSelezionata;
    ProceduraVoto::statiProcedura statoProceduraSelezionata;
    QString descProceduraSelezionata;

signals:
    void attemptLogin(QString username,QString password);
    void startScrutinio(uint idProcedura);
    void needRisultatiVoto(uint idProcedura);
public slots:
    void showProcedureRP(vector <ProceduraVoto> pv);
    void messageUrnaUnreachable();
    void messageWrongCredentials();
    void incrementProgressBar();
    void resizeProgressBar(uint dim);
    void showMessageScrutinioCompletato();
    void errorMessageScrutinio();

private slots:
    void on_pushButton_closeApp_clicked();

    void on_pushButton_doLogin_clicked();

    void on_tableWidget_vistaProcedure_cellClicked(int row, int column);

};

#endif // MAINWINDOWRP_H
