#include<db.h>
#include<QTextStream>
#include<QFile>
#include <QDebug>

//******************************************************************************************
//******************************************************************************************
Db::Db()
{
    activedb = QSqlDatabase::addDatabase("QPSQL","test");
    // qDebug() << activedb.isValid();
}

//******************************************************************************************
//******************************************************************************************
bool Db::createConnection()
{
    activedb.setHostName("localhost");
    activedb.setDatabaseName("qttest");
    activedb.setPort(5432);
    activedb.setUserName("coin");
    activedb.setPassword("coin");
    if (!activedb.open()) {
        // qDebug() << activedb.lastError().text();
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
bool Db::isNewDatabase()
{
    if (activedb.tables().size() > 0) {
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
bool Db::initialDatabase()
{
    QSqlQuery query = QSqlQuery(activedb);
    QString strQuery;
    QFile file(PATH_INIT_FILE);

    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot read file " << PATH_INIT_FILE << "\n";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    strQuery = QString(data);
    if(!query.exec(strQuery)) {
        qDebug() << query.lastError().databaseText();
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addBlock(const std::tuple<int, QString, QString, QDateTime, QString, int> & block)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_block (\"nVersion\", \"hashPrevBlock\", \"hashMerkleRoot\", \"nTime\", \"hash\", \"n\") "
                   "VALUES (:nVer, :hashPrev, :hashMerkle, :nTime, :hash, :n);")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":nVer",        std::get<0>(block));
    q.bindValue(":hashPrev",    std::get<1>(block));
    q.bindValue(":hashMerkle",  std::get<2>(block));
    q.bindValue(":nTime",       std::get<3>(block));
    q.bindValue(":hash",        std::get<4>(block));
    q.bindValue(":n",           std::get<5>(block));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();

    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addTxs(const std::vector<std::tuple<int, unsigned int, QString> >  & txs)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_tx (\"nVersion\", \"nLockTime\", \"hash\") "
                   "VALUES (:nVer, :lockime, :hash);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    for(const std::tuple<int, unsigned int, QString> & tx : txs)
    {
        q.bindValue(":nVer",        std::get<0>(tx));
        q.bindValue(":lockime",       std::get<1>(tx));
        q.bindValue(":hash",        std::get<2>(tx));
    }
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addTx(const std::tuple<int, unsigned int, QString>  & tx)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_tx (\"nVersion\", \"nLockTime\", \"hash\") "
                   "VALUES (:nVer, :lockime, :hash);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    q.bindValue(":nVer",        std::get<0>(tx));
    q.bindValue(":lockime",       std::get<1>(tx));
    q.bindValue(":hash",        std::get<2>(tx));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
bool Db::addBlockVtx(const std::tuple<unsigned int, unsigned int>  & block_vtx)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_block_vtx (\"block_id\", \"tx_id\") "
                   "VALUES (:block_id, :tx_id);")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":block_id",    std::get<0>(block_vtx));
    q.bindValue(":tx_id",       std::get<1>(block_vtx));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
bool Db::addTxVin(const std::tuple<unsigned int, unsigned int>  & tx_vin)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_tx_vin (\"tx_id\", \"txin_id\") "
                   "VALUES (:tx_id, :txin_id);")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":tx_id",         std::get<0>(tx_vin));
    q.bindValue(":txin_id",       std::get<1>(tx_vin));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;

}

//******************************************************************************************
//******************************************************************************************
bool Db::addTxVout(const std::tuple<unsigned int, unsigned int>  & tx_vout)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_tx_vout (\"tx_id\", \"txout_id\") "
                   "VALUES (:tx_id, :txout_id);")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":tx_id",         std::get<0>(tx_vout));
    q.bindValue(":txout_id",      std::get<1>(tx_vout));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;

}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addTxIn(const std::tuple<unsigned int, QString>  & txIn)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_txin (\"prevout_id\", \"scriptSig\") "
                   "VALUES (:prev_id, :script);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    q.bindValue(":prev_id",      std::get<0>(txIn));
    q.bindValue(":script",       std::get<1>(txIn));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addTxOut(const std::tuple<double, QString, int, unsigned int>  & txOut)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_txout (\"nValue\", \"scriptPubKey\", \"n\", \"isSpent\", \"bitcoinAddress_id\") "
                   "VALUES (:nVal, :script, :n, false, :addr);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    q.bindValue(":nVal",         std::get<0>(txOut));
    q.bindValue(":script",       std::get<1>(txOut));
    q.bindValue(":n",            std::get<2>(txOut));
    q.bindValue(":addr",         std::get<3>(txOut));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
bool Db::getTxOutToSpent(const std::tuple<QString, unsigned int> raw,  std::tuple<unsigned int> & idTxOut)
{
    QSqlQuery q(activedb);

    if (!q.prepare("select btc_txout.id, btc_txout.n, btc_tx_vout.tx_id, btc_tx.hash  "
                   "from btc_txout inner join btc_tx_vout on btc_txout.id = btc_tx_vout.txout_id "
                   "inner join btc_tx on btc_tx.id = btc_tx_vout.tx_id and btc_txout.n= :n and btc_tx.hash= :hash;")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":hash",        std::get<0>(raw));
    q.bindValue(":n",           std::get<1>(raw));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    if (q.size()>1) {
        qDebug() << "More TxOut to spent ERROR. Size - " << q.size();
        return false;
    } else if (q.size()==0) {
        qDebug() << "Not Found TxOut to spent. Size - " << q.size();
        if (std::get<1>(raw) > 0) {
            qDebug() << "Return current TxOut for ID - " << std::get<1>(raw);
            idTxOut=std::get<1>(raw);
            return true;
        } else
            return false;
    }
    q.first();
    // qDebug() << q.value(0).toInt() << q.value(1).toInt() << q.value(2).toInt();
    idTxOut=q.value(0).toInt();
    return true;

}

//******************************************************************************************
//******************************************************************************************
bool Db::setTxOutToSpent(const std::tuple<unsigned int> raw)
{
    QSqlQuery q(activedb);

    if (!q.prepare("UPDATE btc_txout SET \"isSpent\" = true "
                   "WHERE \"id\"= :id")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":id",            std::get<0>(raw));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
bool Db::updateOutpointToTxout(const std::tuple<unsigned int, unsigned int> raw)
{
    QSqlQuery q(activedb);

    if (!q.prepare("UPDATE btc_outpoint SET \"n_id\" =:txoutId "
                   "WHERE \"id\"= :id")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":id",            std::get<0>(raw));
    q.bindValue(":txoutId",       std::get<1>(raw));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;

}

//******************************************************************************************
//******************************************************************************************
bool Db::updateIdAddressInTxOut(const std::tuple<unsigned int> & oldAddrId,  const std::tuple<unsigned int> & newAddrId)
{
    QSqlQuery q(activedb);

    if (!q.prepare("UPDATE btc_txout SET \"bitcoinAddress_id\" = :addr "
                   "WHERE \"bitcoinAddress_id\"= :id")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":addr",         std::get<0>(newAddrId));
    q.bindValue(":id",           std::get<0>(oldAddrId));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;
}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addOutPoint(const std::tuple<QString, int>  & outPoint)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_outpoint (\"hash\", \"n_id\") "
                   "VALUES (:hash, :n);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    q.bindValue(":hash",         std::get<0>(outPoint));
    q.bindValue(":n",            std::get<1>(outPoint));

    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
QVariant Db::addBitcoinAddress(const std::tuple<QString>  & address)
{
    QSqlQuery q(activedb);

    if (!q.prepare("INSERT INTO btc_bitcoinaddress (\"address\", \"isUsed\") "
                   "VALUES (:address, false);")) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    q.bindValue(":address",         std::get<0>(address));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return QVariant();
    }
    return q.lastInsertId();

}

//******************************************************************************************
//******************************************************************************************
bool Db::removeBitcoinAddress(const std::tuple<int>  & id)
{
    QSqlQuery q(activedb);

    if (!q.prepare("DELETE FROM btc_bitcoinaddress WHERE id=:id")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    q.bindValue(":id",         std::get<0>(id));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    return true;

}
//******************************************************************************************
//******************************************************************************************
bool Db::getBitcoinAddresses(const std::tuple<QString> str, std::vector<int> & addresses)
{
    QSqlQuery q(activedb);

    if (!q.prepare("SELECT id "
                   "FROM btc_bitcoinaddress "
                   "WHERE address= :addr; ")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
     q.bindValue(":addr",         std::get<0>(str));
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    while (q.next())
    {
        addresses.push_back(q.value(0).toInt());
    }
    return true;

}

//******************************************************************************************
//******************************************************************************************
bool Db::getDublicateAddresses(std::vector<QString> & addresses)
{
    QSqlQuery q(activedb);

    if (!q.prepare("select address, count(*) "
                   "from btc_bitcoinaddress "
                   "group by address "
                   "having count(*) > 1")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    while (q.next())
    {
        addresses.push_back(q.value(0).toString());
    }
    return true;

}

//******************************************************************************************
//******************************************************************************************
bool Db::getAllOutpoint(std::vector<std::tuple<unsigned int, QString, unsigned int> > & outpoints)
{
    QSqlQuery q(activedb);

    if (!q.prepare("SELECT id, hash, n_id "
                   "FROM btc_outpoint "
                   "ORDER BY id;")) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    if (!q.exec()) {
        qDebug() << q.lastError().databaseText();
        return false;
    }
    while (q.next())
    {
        outpoints.push_back(std::make_tuple(q.value(0).toInt(), q.value(1).toString(), q.value(2).toInt()));
    }
    return true;

}
