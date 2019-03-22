#ifndef CONNECTION_H
#define CONNECTION_H

#define DBNAME "qttest"
#define PATH_INIT_FILE "/Users/konstantin/qtcreator/postgresdb/sql_init.txt"
#define PATH_RMDUBLE_FILE "/Users/konstantin/qtcreator/postgresdb/sql_rm_dublicates.txt"
#define DBHOSTNAME "192.168.101.107"

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
    QVariant addTxOut(const std::tuple<double, QString, int, unsigned int>  & txOut);
    bool getTxOutToSpent(const std::tuple<QString, unsigned int> raw,  std::tuple<unsigned int> & idTxOut);
    bool setTxOutToSpent(const std::tuple<unsigned int> raw);
    bool updateIdAddressInTxOut(const std::tuple<unsigned int> & txOutId,  const std::tuple<unsigned int> & newTxOutId);
    QVariant addOutPoint(const std::tuple<QString, int>  & outPoint);
    bool updateOutpointToTxout(const std::tuple<unsigned int, unsigned int> raw);
    QVariant addBitcoinAddress(const std::tuple<QString>  & address);
    bool getBitcoinAddresses(const std::tuple<QString> str, std::vector<int> & addresses);
    bool removeBitcoinAddress(const std::tuple<int>  & id);
    bool getDublicateAddresses(std::vector<QString> & addresses);
    bool getAllOutpoint(std::vector<std::tuple<unsigned int, QString, unsigned int> > & outpoints);
    QVariant getBlockCount();
    bool removeDublicateAddresses();

private:
    QSqlDatabase activedb;

};


#endif // CONNECTION_H
