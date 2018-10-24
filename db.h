#ifndef CONNECTION_H
#define CONNECTION_H

#define PATH_INIT_FILE "/Users/konstantin/qtcreator/postgresdb/sql_init.txt"

#include <iostream>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QDateTime>
#include <vector>

/*
    This file defines the class for working with the database.
*/

class Db
{

public:
    Db();
    ~Db() {
        activedb.close();
        // activedb.removeDatabase("test");
    }
    bool createConnection();
    bool isNewDatabase();
    bool initialDatabase();
    QVariant addBlock(const std::tuple<int, QString, QString, QDateTime, QString, int>  & block);
    QVariant addTxs(const std::vector<std::tuple<int, unsigned int, QString> >  & txs);
    QVariant addTx(const std::tuple<int, unsigned int, QString>  & txs);
    bool addBlockVtx(const std::tuple<unsigned int, unsigned int>  & block_vtx);
    bool addTxVin(const std::tuple<unsigned int, unsigned int>  & tx_vin);
    bool addTxVout(const std::tuple<unsigned int, unsigned int>  & tx_vout);
    QVariant addTxIn(const std::tuple<unsigned int, QString>  & txIn);
    QVariant addTxOut(const std::tuple<unsigned int, QString, int, unsigned int>  & txOut);
    bool updateIdAddressInTxOut(const std::tuple<unsigned int> & txOutId,  const std::tuple<unsigned int> & newTxOutId);
    QVariant addOutPoint(const std::tuple<QString, int>  & outPoint);
    QVariant addBitcoinAddress(const std::tuple<QString>  & address);
    bool getBitcoinAddresses(const std::tuple<QString> str, std::vector<int> & addresses);
    bool removeBitcoinAddress(const std::tuple<int>  & id);
    bool getDublicateAddresses(std::vector<QString> & addresses);

private:
    QSqlDatabase activedb;

};


#endif // CONNECTION_H
