#include "mainwindowrp.h"
#include "ui_mainwindowrp.h"


MainWindowRP::MainWindowRP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowRP)
{
    ui->setupUi(this);
    userAgent = new UserAgentRP(this);
    ui->stackedWidget->setCurrentIndex(InterfacceRP::login);
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);


    qRegisterMetaType< vector<ProceduraVoto> >( "vector<ProceduraVoto>" );
    QObject::connect(this,SIGNAL(attemptLogin(QString,QString)),userAgent,SLOT(login(QString,QString)));
    QObject::connect(this,SIGNAL(startScrutinio(uint)),userAgent,SLOT(doScrutinio(uint)));
    QObject::connect(this,SIGNAL(needRisultatiVoto(uint)),userAgent,SLOT(visualizzaRisultatiVoto(uint)));
    QObject::connect(userAgent,SIGNAL(autenticazione_riuscita(vector<ProceduraVoto>)),this,SLOT(showProcedureRP(vector<ProceduraVoto>)));
    QObject::connect(userAgent,SIGNAL(urnaNonRaggiungibile()),this,SLOT(messageUrnaUnreachable()));
    QObject::connect(userAgent,SIGNAL(errorCredenziali()),this,SLOT(messageWrongCredentials()));
    QObject::connect(userAgent,SIGNAL(oneMore()),this,SLOT(incrementProgressBar()));
    QObject::connect(userAgent,SIGNAL(schedeDaScrutinare(uint)),SLOT(resizeProgressBar(uint)));
    QObject::connect(userAgent,SIGNAL(erroreScrutinio()),this,SLOT(errorMessageScrutinio()));
    QObject::connect(userAgent,SIGNAL(scrutinioOK()),this,SLOT(showMessageScrutinioCompletato()));
}

MainWindowRP::~MainWindowRP()
{
    delete ui;
}

void MainWindowRP::showProcedureRP(vector<ProceduraVoto> pv)
{

}

void MainWindowRP::messageUrnaUnreachable()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Error");
    msgBox.setInformativeText("Impossibile comunicare con l'urna, verificare la connessione");
    msgBox.exec();
}

void MainWindowRP::messageWrongCredentials()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Error");
    msgBox.setInformativeText("Username o password errati, riprovare");
    msgBox.exec();
}

void MainWindowRP::incrementProgressBar()
{
    int value= ui->progressBar_avanzamentoScrutinio->value();
    ui->progressBar_avanzamentoScrutinio->setValue(value+1);
}

void MainWindowRP::resizeProgressBar(uint dim)
{
    ui->progressBar_avanzamentoScrutinio->setMaximum(dim);
}

void MainWindowRP::showMessageScrutinioCompletato()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Success");
    msgBox.setInformativeText("Scrutinio completato con successo.");
    msgBox.exec();
}

void MainWindowRP::errorMessageScrutinio()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Error");
    msgBox.setInformativeText("Errore durante lo scrutinio, l'operazione dovrà essere ripetuta.");
    msgBox.exec();
}

void MainWindowRP::on_pushButton_closeApp_clicked()
{
    QApplication::quit();
}

void MainWindowRP::on_pushButton_doLogin_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    if(username=="" || password==""){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error");
        msgBox.setInformativeText("Inserire username e password per accedere");
        msgBox.exec();
        return;
    }
    emit attemptLogin(username,password);
}

void MainWindowRP::on_tableWidget_vistaProcedure_cellClicked(int row, int column)
{
    unsigned int currentRow = row;
    if(column==0){
        if(ui->tableWidget_vistaProcedure->item(row,0)->checkState() == Qt::Checked){

            idProceduraSelezionata = ui->tableWidget_vistaProcedure->item(currentRow,1)->text().toUInt();
            QString stato = ui->tableWidget_vistaProcedure->item(currentRow,8)->text();
            statoProceduraSelezionata = ProceduraVoto::getStatoFromString(stato.toStdString());
            descProceduraSelezionata = ui->tableWidget_vistaProcedure->item(currentRow,2)->text();
            cout << "id Procedura selezionata: " << idProceduraSelezionata << ", stato: " << ProceduraVoto::getStatoAsString(statoProceduraSelezionata) << endl;
            unsigned int numberRows = ui->tableWidget_vistaProcedure->rowCount();

            //se abbiamo selezionato una riga diversa dalla precedente, deselezioniamo qualsiasi selezione precedente
            for (unsigned int rowIndex = 0; rowIndex < numberRows; rowIndex++){
                if(rowIndex!=currentRow){
                    ui->tableWidget_vistaProcedure->item(rowIndex,0)->setCheckState(Qt::Unchecked);
                }
            }

            switch(statoProceduraSelezionata){
            case ProceduraVoto::statiProcedura::creazione:
            case ProceduraVoto::statiProcedura::programmata:
            case ProceduraVoto::statiProcedura::da_eliminare:
            case ProceduraVoto::statiProcedura::in_corso:
                ui->pushButton_avviaScrutinio->setEnabled(false);
                ui->pushButton_visualizzaRisultati->setEnabled(false);
                break;
            case ProceduraVoto::statiProcedura::conclusa:
                ui->pushButton_avviaScrutinio->setEnabled(true);
                ui->pushButton_visualizzaRisultati->setEnabled(false);
                break;
            case ProceduraVoto::statiProcedura::scrutinata:
                ui->pushButton_avviaScrutinio->setEnabled(true);
                ui->pushButton_visualizzaRisultati->setEnabled(true);
                break;
            default:
                break;

            }


        }
        else if(ui->tableWidget_vistaProcedure->item(row,0)->checkState() == Qt::Unchecked){
            int idProceduraDeselezionata = ui->tableWidget_vistaProcedure->item(currentRow,1)->text().toUInt();
            if(idProceduraDeselezionata == idProceduraSelezionata){
                ui->pushButton_avviaScrutinio->setEnabled(false);
                ui->pushButton_visualizzaRisultati->setEnabled(false);
                idProceduraSelezionata = -1;
                statoProceduraSelezionata = ProceduraVoto::statiProcedura::undefined;
                cout << "nessuna Procedura selezionata! " << endl;
            }
        }
    }
}


