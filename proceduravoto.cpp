#include "proceduravoto.h"
#include <iostream>
using namespace std;

ProceduraVoto::ProceduraVoto()
{
    this->data_ora_inizio = "";
    this->data_ora_termine = "";
}

string ProceduraVoto::getDescrizione() const
{
    return descrizione;
}

void ProceduraVoto::setDescrizione(const string &value)
{
    descrizione = value;
}

string ProceduraVoto::getData_ora_inizio() const
{
    return data_ora_inizio;
}

void ProceduraVoto::setData_ora_inizio(const string &value)
{
    data_ora_inizio = value;
}

string ProceduraVoto::getData_ora_termine() const
{
    return data_ora_termine;
}

void ProceduraVoto::setData_ora_termine(const string &value)
{
    data_ora_termine = value;
}

uint ProceduraVoto::getIdProceduraVoto() const
{
    return idProceduraVoto;
}

void ProceduraVoto::setIdProceduraVoto(const uint &value)
{
    idProceduraVoto = value;
}


ProceduraVoto::statiProcedura ProceduraVoto::getStato() const
{
    return stato;
}

void ProceduraVoto::setStato(const statiProcedura &value)
{
    stato = value;
}

void ProceduraVoto::setStato(const uint &stato){
    switch(stato){
    case ProceduraVoto::statiProcedura::creazione:
        this->stato =  ProceduraVoto::statiProcedura::creazione;
        break;
    case ProceduraVoto::statiProcedura::programmata:
        this->stato =  ProceduraVoto::statiProcedura::programmata;
        break;
    case ProceduraVoto::statiProcedura::in_corso:
        this->stato =  ProceduraVoto::statiProcedura::in_corso;
        break;
    case ProceduraVoto::statiProcedura::conclusa:
        this->stato =  ProceduraVoto::statiProcedura::conclusa;
        break;
    case ProceduraVoto::statiProcedura::scrutinata:
            this->stato =  ProceduraVoto::statiProcedura::scrutinata;
            break;
    case ProceduraVoto::statiProcedura::da_eliminare:
        this->stato =  ProceduraVoto::statiProcedura::da_eliminare;
        break;
    default:
        this->stato =  ProceduraVoto::statiProcedura::undefined;
        break;

    }

}

string ProceduraVoto::getStatoAsString(ProceduraVoto::statiProcedura stato)
{
    string statoAsString;
    switch(stato){
    case creazione:
        statoAsString = "creazione";
        break;
    case programmata:
        statoAsString = "programmata";
        break;
    case in_corso:
        statoAsString = "in corso";
        break;
    case conclusa:
        statoAsString = "conclusa";
        break;
    case scrutinata:
        statoAsString = "scrutinata";
        break;
    case da_eliminare:
        statoAsString = "da eliminare";
        break;
    case undefined:
        statoAsString = "undefined";
        break;
    }

    return statoAsString;
}


ProceduraVoto::statiProcedura ProceduraVoto::getStatoFromString(string stato)
{
    if(stato == "creazione"){
        return statiProcedura::creazione;
    }
    if(stato == "programmata"){
        return statiProcedura::programmata;
    }
    if(stato == "in corso"){
        return statiProcedura::in_corso;
    }
    if(stato == "conclusa"){
        return statiProcedura::conclusa;
    }
    if(stato == "scrutinata"){
        return statiProcedura::scrutinata;
    }
    else{
        return statiProcedura::undefined;
    }
}


