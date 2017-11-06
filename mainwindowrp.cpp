#include "mainwindowrp.h"
#include "ui_mainwindowrp.h"


MainWindowRP::MainWindowRP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowRP)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    setWindowTitle("Voto digitale UNIPA");

    userAgent = new UserAgentRP(this);
    ui->stackedWidget->setCurrentIndex(InterfacceRP::login);
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    this->setTables();

    qRegisterMetaType< vector<ProceduraVoto> >( "vector<ProceduraVoto>" );
    qRegisterMetaType<vector<RisultatiSeggio>>("vector<RisultatiSeggio>");
    QObject::connect(this,SIGNAL(attemptLogin(QString,QString)),userAgent,SLOT(doLogin(QString,QString)));
    QObject::connect(this,SIGNAL(startScrutinio(uint)),userAgent,SLOT(doScrutinio(uint)));
    QObject::connect(this,SIGNAL(needRisultatiVoto(uint)),userAgent,SLOT(visualizzaRisultatiVoto(uint)));
    QObject::connect(userAgent,SIGNAL(readyRisultatiSeggi(vector<RisultatiSeggio>)),this,SLOT(showRisultatiProcedura(vector<RisultatiSeggio>)));
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
    ui->lineEdit_password->clear();
    ui->lineEdit_username->clear();
    //pulizia tabella - tipo 1
    ui->tableWidget_vistaProcedure->model()->removeRows(0,ui->tableWidget_vistaProcedure->rowCount());

    idProceduraSelezionata = -1;
    statoProceduraSelezionata = ProceduraVoto::statiProcedura::undefined;
    ui->pushButton_avviaScrutinio->setEnabled(false);
    ui->pushButton_visualizzaRisultati->setEnabled(false);

    //pulizia tabella - tipo 2
    //    ui->tableWidget_vistaProcedure->clear();
    //    ui->tableWidget_vistaProcedure->setRowCount(0);
    if (pv.size() == 0){
        ui->stackedWidget->setCurrentIndex(InterfacceRP::procedure);
        QMessageBox msgBox(this);

        msgBox.setInformativeText("Non è presente alcuna procedura di cui sei responsabile.");
        msgBox.exec();

        return;
    }
    for (uint row = 0; row < pv.size();row++){
        uint indexNewRow = ui->tableWidget_vistaProcedure->rowCount();
        cout << "indice nuova riga: " << indexNewRow << endl;
         ui->tableWidget_vistaProcedure->insertRow(indexNewRow);
        int rigaAggiunta = ui->tableWidget_vistaProcedure->rowCount()-1;

        QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
        checkBoxItem->setToolTip("seleziona procedura della riga corrispondente");
        checkBoxItem->setCheckState(Qt::Unchecked);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,0,checkBoxItem);

        uint idProcedura = pv.at(row).getIdProceduraVoto();
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(idProcedura));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setTextColor(Qt::black);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,1,item);

        QString descrizione = QString::fromStdString(pv.at(row).getDescrizione());
        item = new QTableWidgetItem(descrizione);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setTextColor(Qt::black);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,2,item);

        QString qsInizio = QString::fromStdString(pv.at(row).getData_ora_inizio());
        item = new QTableWidgetItem(qsInizio);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setTextColor(Qt::black);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,3,item);

        QString qsTermine = QString::fromStdString(pv.at(row).getData_ora_termine());
        item = new QTableWidgetItem(qsTermine);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setTextColor(Qt::black);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,4,item);

        ProceduraVoto::statiProcedura statoProcedura = pv.at(row).getStato();
        QString stato = QString::fromStdString(ProceduraVoto::getStatoAsString(statoProcedura));
        item = new QTableWidgetItem(stato);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setTextColor(Qt::black);
        ui->tableWidget_vistaProcedure->setItem(rigaAggiunta,5,item);

    }
    ui->tableWidget_vistaProcedure->resizeColumnsToContents();
    ui->tableWidget_vistaProcedure->horizontalHeader()->setStretchLastSection(true);


    ui->stackedWidget->setCurrentIndex(InterfacceRP::procedure);
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

void MainWindowRP::showRisultatiProcedura(vector<RisultatiSeggio> risultatiSeggi)
{
    risultatiSeggioOttenuti = risultatiSeggi;
    initTableRV();


    indexSchedaRisultatoDaMostrare = 0;
    RisultatiSeggio rs = risultatiSeggioOttenuti.at(0);
    uint numSchede = rs.getSchedeVotoRisultato().size();
    ui->stackedWidget->setCurrentIndex(InterfacceRP::risultati);
    if(numSchede > 1 ){
        ui->pushButton_schedaSuccessiva->setEnabled(true);
    }
    else{
        ui->pushButton_schedaSuccessiva->setEnabled(false);
    }
    showSchedaRisultato(indexSchedaRisultatoDaMostrare, risultatiSeggioOttenuti);
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
            int tempID = ui->tableWidget_vistaProcedure->item(currentRow,1)->text().toInt();
            if(tempID!=idProceduraSelezionata){
                idProceduraSelezionata = tempID;
                idProceduraSelezionata = ui->tableWidget_vistaProcedure->item(currentRow,1)->text().toUInt();
                QString stato = ui->tableWidget_vistaProcedure->item(currentRow,5)->text();
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


void MainWindowRP::setTables(){
    QStringList tableHeaders;
    tableHeaders << "seleziona" << "id Procedura Voto" << "Descrizione" << "Inizio" << "Termine"  << "Stato Procedura" ;

    ui->tableWidget_vistaProcedure->setHorizontalHeaderLabels(tableHeaders);
    ui->tableWidget_vistaProcedure->resizeColumnsToContents();
    ui->tableWidget_vistaProcedure->setEditTriggers(QAbstractItemView::NoEditTriggers);


}

void MainWindowRP::on_pushButton_avviaScrutinio_clicked()
{
    emit startScrutinio(idProceduraSelezionata);
}

void MainWindowRP::on_pushButton_logout_clicked()
{
    //TODO comunica all'urna che stai effettuando il logout

    cout << "Logout" << endl;
    ui->stackedWidget->setCurrentIndex(InterfacceRP::login);
}

void MainWindowRP::on_pushButton_visualizzaRisultati_clicked()
{
    emit needRisultatiVoto(idProceduraSelezionata);
}

void MainWindowRP::initTableRV(){
    ui->tableWidget_risultatiVoto->verticalHeader()->setVisible(false);
    ui->tableWidget_risultatiVoto->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_risultatiVoto->setSelectionMode(QAbstractItemView::NoSelection);
    QStringList tableHeaders;
    tableHeaders << "Seggio" << "Candidato" << "Data di nascita" << "Luogo di Nascita " << "Voti Ricevuti" << "Lista" ;

    ui->tableWidget_risultatiVoto->setColumnCount(6);
    ui->tableWidget_risultatiVoto->setHorizontalHeaderLabels(tableHeaders);
    //ui->tableWidget_lista_procedure->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_risultatiVoto->resizeColumnsToContents();
    ui->tableWidget_risultatiVoto->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->tableWidget_risultatiVoto->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    //    ui->tableWidget_risultatiVoto->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    QFont serifFont("Times", 14, QFont::Bold);
    ui->tableWidget_risultatiVoto->horizontalHeader()->setFont( serifFont );

}

void MainWindowRP::showSchedaRisultato(uint indexScheda, vector <RisultatiSeggio> &risultatiSeggi){
    ui->tableWidget_risultatiVoto->model()->removeRows(0,ui->tableWidget_risultatiVoto->rowCount());

    uint indexNewRow, addedRow;


    QFont serifFontSeggio("Times", 20, QFont::Bold);
    QFont serifFontCandidato("Times", 14);

    for (uint s = 0; s <risultatiSeggi.size(); s++){
        indexNewRow = ui->tableWidget_risultatiVoto->rowCount();
        ui->tableWidget_risultatiVoto->insertRow(indexNewRow);
        addedRow = ui->tableWidget_risultatiVoto->rowCount()-1;
        //cout << "Riga aggiunta index: " << addedRow << endl;
        //ottengo i risultati del seggio
        RisultatiSeggio rs = risultatiSeggi.at(s);

        uint idSeggio = rs.getIdSeggio();
        QTableWidgetItem *itemIdSeggio;
        if(idSeggio != 0){
            itemIdSeggio  = new QTableWidgetItem("Seggio " + QString::number(idSeggio));
            cout << "Visualizzazione risultati seggio: " << idSeggio << endl;
        }
        else{
            itemIdSeggio  = new QTableWidgetItem("Urna Virtuale");
            cout << "Visualizzazione risultati Urna Virtuale Centrale" << endl;
        }
        itemIdSeggio->setFont(serifFontSeggio);
        itemIdSeggio->setTextColor(Qt::blue);
        ui->tableWidget_risultatiVoto->setItem(addedRow,SEGGIO,itemIdSeggio);


        //estraggo la scheda risultato da visualizzare
        SchedaVoto sv = rs.getSchedeVotoRisultato().at(indexScheda);
        vector <ListaElettorale> liste = sv.getListeElettorali();
        for (uint l = 0; l< liste.size(); l++){
            ListaElettorale lista = liste.at(l);
            vector <Candidato> candidati = lista.getCandidati();

            for (uint i = 0; i < candidati.size(); i++){
                indexNewRow = ui->tableWidget_risultatiVoto->rowCount();
                ui->tableWidget_risultatiVoto->insertRow(indexNewRow);
                addedRow = ui->tableWidget_risultatiVoto->rowCount()-1;
                //cout << "Riga aggiunta index: " << addedRow << endl;
                QTableWidgetItem *item;
                QString nomeLista = QString::fromStdString(lista.getNome());
                if(nomeLista!="nessuna lista"){
                    //creazione item nome della lista di appartenenza del candidato
                    item = new QTableWidgetItem(nomeLista);
                    item->setFont(serifFontCandidato);
                    item->setTextColor(Qt::black);
                    ui->tableWidget_risultatiVoto->setItem(addedRow,LISTA,item);
                }

                Candidato c = candidati.at(i);

                string candidato = c.getNome() + " " +c.getCognome();
                item = new QTableWidgetItem(QString::fromStdString(candidato));
                item->setFont(serifFontCandidato);
                item->setTextColor(Qt::black);
                ui->tableWidget_risultatiVoto->setItem(addedRow,CANDIDATO,item);

                string luogo = c.getLuogoNascita();
                item = new QTableWidgetItem(QString::fromStdString(luogo));
                item->setFont(serifFontCandidato);
                item->setTextColor(Qt::black);
                ui->tableWidget_risultatiVoto->setItem(addedRow,LUOGO,item);

                string data = c.getDataNascita();
                item = new QTableWidgetItem(QString::fromStdString(data));
                item->setFont(serifFontCandidato);
                item->setTextColor(Qt::black);
                ui->tableWidget_risultatiVoto->setItem(addedRow,DATA,item);

                string numVoti = to_string(c.getNumVoti());
                item = new QTableWidgetItem(QString::fromStdString(numVoti));
                item->setFont(serifFontCandidato);
                item->setTextColor(Qt::black);
                ui->tableWidget_risultatiVoto->setItem(addedRow,NUM_VOTI,item);


                ui->tableWidget_risultatiVoto->resizeColumnsToContents();
            }
        }
    }
}


void MainWindowRP::on_pushButton_schedaSuccessiva_clicked()
{
    RisultatiSeggio rs = risultatiSeggioOttenuti.at(0);
    uint numSchede = rs.getSchedeVotoRisultato().size();
    uint indexSchedaRisultatoMostrata = indexSchedaRisultatoDaMostrare;
    if(indexSchedaRisultatoMostrata < (numSchede-1) ){
        indexSchedaRisultatoDaMostrare++;
    }
    else{ //visualizziamo di nuovo la prima scheda
        indexSchedaRisultatoDaMostrare = 0;
    }
    showSchedaRisultato(indexSchedaRisultatoDaMostrare, risultatiSeggioOttenuti);
}

void MainWindowRP::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(InterfacceRP::procedure);
}


