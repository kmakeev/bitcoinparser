#include<QCoreApplication>
#include<QTextStream>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>

#include"httpclient.h"
#include"db.h"

#define COIN 100000000

static Db db;

void help(QString patch) {
    QTextStream cout(stdout);
    cout << "usage: " << patch  << "\n";
    cout << "                       [-p ] <first block number> <last block number> - to limit database parsing in the specified range \n";
    cout << "                       [-b ] <hash> - to parsing block witch <hash>. If this block is the last, Outpoint will by spent \n";
    cout << "                       [-r ] - to remove dublacate adresses in database \n";
    cout << "                       [-s ] - to spent Outpoint transaction inputs \n";
}

QVariant parseBlock(QJsonValue & result, int c, bool witchSpent=false){
    QString hashPrevBlock;
    if (c == 0)
        hashPrevBlock = "0000000000000000000000000000000000000000000000000000000000000000";
    else
        hashPrevBlock = result.toObject().value("previousblockhash").toString();

    auto block = std::make_tuple(result.toObject().value("version").toInt(), hashPrevBlock,
                                 QDateTime::fromTime_t(result.toObject().value("time").toInt()),
                                 result.toObject().value("hash").toString(), result.toObject().value("height").toInt());

    QVariant block_id = db.addBlock(block);
    if (!block_id.isValid()){
        qDebug() << "Write in DB record of block has ERROR!";
        return QVariant();
    }
    // qDebug() << "Greated Block record IDs" << block_id;

    // std::vector<std::tuple<int, unsigned int, QString> > transactions;
    QJsonArray txs = result.toObject().value("tx").toArray();
    for (int i=0; i < txs.count(); i++){
        // qDebug() << txs[i];
        QVariant tx_id = db.addTx(std::make_tuple(txs[i].toObject().value("version").toInt(),
                                                  txs[i].toObject().value("locktime").toInt(),
                                                  txs[i].toObject().value("hash").toString()));
        if (!tx_id.isValid()){
            qDebug() << "Write in DB record of tx has ERROR!";
            return QVariant();
        }
        // qDebug() << "Greated TX record ID" << tx_id;
        if (!db.addBlockVtx(std::make_tuple(block_id.toInt(), tx_id.toInt()))){
            qDebug() << "Write in DB record of block_tx has ERROR!";
            return QVariant();
        }
        QJsonArray txvin = txs[i].toObject().value("vin").toArray();
        for(int j=0; j < txvin.count(); j++) {
            // qDebug() << txvin[j].toObject();
            if (!txvin[j].toObject().value("coinbase").isUndefined()){
                // qDebug() << "Coinbase TxIn detect";

                // QVariant txvin_id = db.addTxIn(std::make_tuple(txvin[j].toObject().value("vout").toInt(),
                //                                               txvin[j].toObject().value("txid").toString()));
            } else {

                QVariant outpoint_id = db.addOutPoint(std::make_tuple(txvin[j].toObject().value("txid").toString(),
                                                                      txvin[j].toObject().value("vout").toInt()));
                if(!outpoint_id.isValid()) {
                    qDebug() << "Write in DB record of outpoint has ERROR!";
                    return QVariant();
                }
                if (witchSpent) {
                    std::tuple<unsigned int> idTxOut;
                    if (!db.getTxOutToSpent(std::make_tuple(txvin[j].toObject().value("txid").toString(),
                                                            txvin[j].toObject().value("vout").toInt()), idTxOut)) {
                        qDebug() << "Get TxOut has ERROR!";
                        return QVariant();
                    }
                    if (!db.updateOutpointToTxout(std::make_tuple(outpoint_id.toInt(), std::get<0>(idTxOut)))) {
                        qDebug() << "UPDATE OutPoint on new ID TxOut has ERROR!";
                        return QVariant();
                    }
                }
                QVariant txvin_id = db.addTxIn(std::make_tuple(outpoint_id.toInt(),
                                                               txvin[j].toObject().value("scriptSig").toObject().value("hex").toString()));
                if(!txvin_id.isValid()) {
                    qDebug() << "Write in DB record of vin has ERROR!";
                    return QVariant();
                }
                if (!db.addTxVin(std::make_tuple(tx_id.toInt(), txvin_id.toInt()))){
                    qDebug() << "Write in DB record of tx_txin has ERROR!";
                    return QVariant();
                }
            }
        }
        QJsonArray txvout = txs[i].toObject().value("vout").toArray();
        for(int j=0; j < txvout.count(); j++) {
            // qDebug() << txvout[j].toObject();
            QJsonArray addresses = txvout[j].toObject().value("scriptPubKey").toObject().value("addresses").toArray();
            // qDebug() << "Addresses in tx_out" << addresses;
            QVariant addr_id;
            QString addr;
            if (addresses.count() > 0) {
                addr = addresses[0].toString();
            } else {
                addr = "UNDEFINED";
            }
            if(witchSpent){
                std::vector<int> addresses;
                addresses.clear();
                if (!db.getBitcoinAddresses(std::make_tuple(addr), addresses)){
                    qDebug() << "Read bitcoinaddresses list from DB has ERROR!";
                    return QVariant();
                }
                if (addresses.size() > 1){
                    for (int i=1; i < addresses.size(); i++){
                        // qDebug() << addr[i];
                        if (!db.updateIdAddressInTxOut(std::make_tuple(addresses[i]), std::make_tuple(addresses[0]))) {
                            qDebug() << "UPDATE TxOut on new ID bitcoinAddress has ERROR!";
                            return QVariant();
                        }
                        if (!db.removeBitcoinAddress(std::make_tuple(addresses[i]))) {
                            qDebug() << "DELETE bitcoinaddresses from DB has ERROR!";
                            return QVariant();
                        }
                    }
                    addr_id = QVariant(addresses[0]);
                } else {
                    addr_id = db.addBitcoinAddress(std::make_tuple(addr));
                    // qDebug() << "Greated BITCOINADDRES record ID" << addr_id;
                    if(!addr_id.isValid()) {
                        qDebug() << "Write in DB record of Address has ERROR!";
                        return QVariant();
                    }
                }
            } else {
                addr_id = db.addBitcoinAddress(std::make_tuple(addr));
                // qDebug() << "Greated BITCOINADDRES record ID" << addr_id;
                if(!addr_id.isValid()) {
                    qDebug() << "Write in DB record of Address has ERROR!";
                    return QVariant();
                }
            }
            // qDebug() << "nValue" << txvout[j].toObject().value("value");
            QVariant txvout_id = db.addTxOut(std::make_tuple(txvout[j].toObject().value("value").toDouble() * COIN,
                                                             txvout[j].toObject().value("scriptPubKey").toObject().value("hex").toString(),
                                                             txvout[j].toObject().value("n").toInt(),
                                                             addr_id.toInt()));
            if(!txvout_id.isValid()) {
                qDebug() << "Write in DB record of vin has ERROR!";
                return QVariant();
            }
            if (!db.addTxVout(std::make_tuple(tx_id.toInt(), txvout_id.toInt()))){
                qDebug() << "Write in DB record of tx_txout has ERROR!";
                return QVariant();
            }

        }
    }
    return block_id;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream cout(stdout);
    QJsonArray params;
    int startBlockNumber = -1, finishBlockNumber = -1;
    int mode;

    int countArguments = a.arguments().size();
    if (countArguments < 2) {
        help(argv[0]);
        return EXIT_FAILURE;
    }
    if (a.arguments().contains("-p") && a.arguments().indexOf("-p") == 1) {
        switch (countArguments) {
        case 2:
            break;
        case 3:
            startBlockNumber = a.arguments().at(2).toInt();
            break;
        case 4:
            startBlockNumber = a.arguments().at(2).toInt();
            finishBlockNumber = a.arguments().at(3).toInt();
            if (startBlockNumber > finishBlockNumber) {
                help(argv[0]);
                return EXIT_FAILURE;
            }
            break;
        default:
            cout << "wrong parameters specified \n";
            help(argv[0]);
            return EXIT_FAILURE;
        }
        mode = 1;
    } else if (a.arguments().contains("-r") && a.arguments().indexOf("-r") == 1) {
        mode = 2;
    } else if (a.arguments().contains("-s") && a.arguments().indexOf("-s") == 1) {
        mode = 3;
    } else if (a.arguments().contains("-b") && a.arguments().indexOf("-b") == 1) {
        mode = 4;
    } else {
        help(argv[0]);
        return EXIT_FAILURE;
    }
    if (!db.createConnection()) {
        cout << "Cannot open database \n";
        return EXIT_FAILURE;
    }
    if(db.isNewDatabase()) {
        cout << "It is New Database \n";
        if(!db.initialDatabase()) {
            cout << "Cannot initial database \n";
            return EXIT_FAILURE;
        }
        cout << "Database initialized.\n";
    } else {
        cout << "Вatabase is not empty.\n";
    }
    HttpClient client(RPCHOST);
    switch (mode) {
    case 1:
        //client.setUsername("plc");
        // client.setPassword("plc");
        if (finishBlockNumber <= -1) {
            QVariant currentBlockCount = client.getLastBlockNumber().toInt();
            if (!currentBlockCount.isValid()) {
                qDebug() << "Can not get current blockcount witch RPC";
                return EXIT_FAILURE;
            }
            finishBlockNumber = currentBlockCount.toInt();
        }
        if (startBlockNumber > finishBlockNumber) {
            cout << "first block number could not be more last block number \n" << startBlockNumber << "not more" << finishBlockNumber;
            cout << "usage: " << argv[0] << "[-p ] <first block number> <last block number> - to limit database parsing in the specified range \n";
            return EXIT_FAILURE;
        }
        cout << "Begin \n";
        for (int c = startBlockNumber; c <= finishBlockNumber; c++)
        {
            cout << "\r" << c;
            QVariant hash = client.getBlockHash(c);
            if (!hash.isValid()) {
                qDebug() << "Can not get hash witch RPC" << "for block number " << c;
                return EXIT_FAILURE;

            }
            QVariant response = client.getBlock(hash.toString());
            if (!response.isValid()) {
                qDebug() << "Can not get hash witch RPC" << "for block number " << c;
                return EXIT_FAILURE;
            }
            // qDebug() << response.toByteArray();

            QJsonValue result = QJsonDocument::fromVariant(response).object().value("result");
            // qDebug() << result;
            if (!parseBlock(result, c).isValid()) {
                    return EXIT_FAILURE;
            }
        }
        qDebug() <<  "\n FINISH!";
        break;
    case 2: {
        qDebug() << "Remove is starting. Please Wait!!";
        if (db.removeDublicateAddresses()) {
            qDebug() << "Remove completed!!!";
        } else {
            qDebug() << "An error occurred while removing duplicate addresses!";
            return EXIT_FAILURE;
        }
        break;
    }
    case 3: {
        qDebug() << "Start Reading all OutPoint. Please Wait!!";

        if (db.spentAllOutpoints()) {
            qDebug() << "Remove completed!!!";
        } else {
            qDebug() << "An error occurred while removing duplicate addresses!";
            return EXIT_FAILURE;
        }
        break;
    }
    case 4: {
        QVariant currentBlockCount = db.getBlockCount();
        if (!currentBlockCount.isValid()) {
            qDebug() << "Can not get current blockcount in Database";
            return EXIT_FAILURE;
        }
        int lastBlockNumber = currentBlockCount.toInt();
        QString blockhash = a.arguments().at(2);
        QVariant response = client.getBlock(blockhash);
        if (!response.isValid()) {
            qDebug() << "Can not get hash witch RPC" << blockhash;
            return EXIT_FAILURE;
        }
        QJsonValue result = QJsonDocument::fromVariant(response).object().value("result");
        int currentBlockNumber = result.toObject().value("height").toInt();
        bool needRebuild = true;
        bool isLast = true;
        if (lastBlockNumber < currentBlockNumber) {
            qDebug() << "Could not determine further action for block with hash" << blockhash;
            return EXIT_FAILURE;
        }
        if ((lastBlockNumber - currentBlockNumber) > 5) {
            isLast = false;
            needRebuild = false;
        } else if (lastBlockNumber == currentBlockNumber) {
            needRebuild = false;
        } else if ((lastBlockNumber - currentBlockNumber) < 5) {
            isLast = false;
            needRebuild = true;

        } else {
            qDebug() << "Could not determine further action for block with hash" << blockhash;
            return EXIT_FAILURE;
        }
        if (!needRebuild && !isLast) {
            if (!parseBlock(result, currentBlockNumber).isValid()) {
                    return EXIT_FAILURE;
            }
             qDebug() <<  "\nBlock paring";
        } else if (!needRebuild && isLast) {
            QVariant block_id = parseBlock(result, currentBlockNumber, true);
            if (!block_id.isValid()) {
                    qDebug() << "An error occurred while parse block!" << currentBlockNumber;
                    return EXIT_FAILURE;
            }
            qDebug() <<  "\nBlock paring witch spent outpoints!";
        }
        else {
           qDebug() <<  "\nNeed rebuld...";
        }
        break;
    }
    default:
        break;
    }
    return EXIT_SUCCESS;
}
