#ifndef PROCEDURAVOTO_H
#define PROCEDURAVOTO_H
#include <string>
#include <vector>

#include <QtCore>

using namespace std;
class ProceduraVoto
{
public:
    ProceduraVoto();

    string getDescrizione() const;
    void setDescrizione(const string &value);

    string getData_ora_inizio() const;
    void setData_ora_inizio(const string &value);

    string getData_ora_termine() const;
    void setData_ora_termine(const string &value);

    uint getIdProceduraVoto() const;
    void setIdProceduraVoto(const uint &value);

    enum statiProcedura{
        creazione,
        programmata,
        in_corso,
        conclusa,
        scrutinata,
        da_eliminare,
        undefined
    };

    statiProcedura getStato() const;
    void setStato(const statiProcedura &value);
    void setStato(const uint &stato);
    static string getStatoAsString(statiProcedura stato);
    static statiProcedura getStatoFromString(string stato);


private:
    string descrizione;
    statiProcedura stato;
    string data_ora_inizio; //format: yyyy/MM/dd hh:mm
    string data_ora_termine; //format: yyyy/MM/dd hh:mm
    uint idProceduraVoto;


};
Q_DECLARE_METATYPE(ProceduraVoto)
#endif // PROCEDURAVOTO_H
