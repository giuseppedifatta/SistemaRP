#ifndef MAINWINDOWRP_H
#define MAINWINDOWRP_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtCore>
#include "useragentrp.h"
#include "risultatiSeggio.h"

#include "proceduravoto.h"

#include <vector>
using namespace std;
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
    enum columnRisultatiVoto{
        SEGGIO,
        CANDIDATO,
        DATA,
        LUOGO,
        NUM_VOTI,
        LISTA
    };

private:
    Ui::MainWindowRP *ui;
    UserAgentRP * userAgent;

    int idProceduraSelezionata;
    ProceduraVoto::statiProcedura statoProceduraSelezionata;
    QString descProceduraSelezionata;

    vector <RisultatiSeggio> risultatiSeggioOttenuti;
    uint indexSchedaRisultatoDaMostrare;

    void setTables();
    void initTableRV();
    void showSchedaRisultato(uint indexScheda, vector<RisultatiSeggio> &risultatiSeggi);
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
    void showRisultatiProcedura(vector<RisultatiSeggio> risultatiSeggi);

private slots:
    void on_pushButton_closeApp_clicked();
    void on_pushButton_doLogin_clicked();
    void on_tableWidget_vistaProcedure_cellClicked(int row, int column);
    void on_pushButton_avviaScrutinio_clicked();
    void on_pushButton_logout_clicked();
    void on_pushButton_visualizzaRisultati_clicked();
//    void on_pushButton_schedaSuccessiva_clicked();
    void on_pushButton_clicked();
    void on_pushButton_schedaSuccessiva_clicked();
};

#endif // MAINWINDOWRP_H
