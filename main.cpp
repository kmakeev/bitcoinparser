#include<QCoreApplication>
#include<QTextStream>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>

#include"httpclient.h"
#include"db.h"

#define COIN 100000000


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream cout(stdout);
    Db db;
    QJsonDocument doc, doc_tx;
    QJsonArray params;
    int startBlockNumber = -1, finishBlockNumber = -1;

    int countArguments = a.arguments().size();
    if (countArguments < 2) {
        cout << "usage: " << argv[0] << "[-p ] <first block number> <last block number> - to limit database parsing in the specified range \n";
        cout << "";
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
                cout << "first block number could not be more last block number \n";
                cout << "usage: " << argv[0] << "[-p ] <first block number> <last block number> - to limit database parsing in the specified range \n";
                return EXIT_FAILURE;
            }
            break;
        default:
            cout << "wrong parameters specified \n";
            cout << "usage: " << argv[0] << "[-p ] <first block number> <last block number> - to limit database parsing in the specified range \n";
            return EXIT_FAILURE;
        }
    } else {
        cout << "usage: " << argv[0] << "[-p ] <first block number> <last block number> - to limit database parsing in the specified range";
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
        cout << "Database initialized \n";
    } else {
        cout << "Вatabase is not empty. Data parsing continued \n";
    }
    HttpClient client("http://192.168.101.108:8332");
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
        QString hashPrevBlock;
        if (c == 0)
            hashPrevBlock = "0000000000000000000000000000000000000000000000000000000000000000";
        else
            hashPrevBlock = result.toObject().value("previousblockhash").toString();

        auto block = std::make_tuple(result.toObject().value("version").toInt(), hashPrevBlock,
                                     result.toObject().value("merkleroot").toString(), QDateTime::fromTime_t(result.toObject().value("time").toInt()),
                                     result.toObject().value("hash").toString(), result.toObject().value("height").toInt());

        QVariant block_id = db.addBlock(block);
        if (!block_id.isValid()){
            qDebug() << "Write in DB record of block has ERROR!";
            return EXIT_FAILURE;
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
                return EXIT_FAILURE;
            }
            // qDebug() << "Greated TX record ID" << tx_id;
            if (!db.addBlockVtx(std::make_tuple(block_id.toInt(), tx_id.toInt()))){
                qDebug() << "Write in DB record of block_tx has ERROR!";
                return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
                    }

                    QVariant txvin_id = db.addTxIn(std::make_tuple(outpoint_id.toInt(),
                                                                   txvin[j].toObject().value("scriptSig").toObject().value("hex").toString()));
                    if(!txvin_id.isValid()) {
                        qDebug() << "Write in DB record of vin has ERROR!";
                        return EXIT_FAILURE;
                    }
                    if (!db.addTxVin(std::make_tuple(tx_id.toInt(), txvin_id.toInt()))){
                        qDebug() << "Write in DB record of tx_txin has ERROR!";
                        return EXIT_FAILURE;
                    }
                }

            }
            QJsonArray txvout = txs[i].toObject().value("vout").toArray();
            for(int j=0; j < txvout.count(); j++) {
                // qDebug() << txvout[j].toObject();
                QJsonArray addresses = txvout[j].toObject().value("scriptPubKey").toObject().value("addresses").toArray();
                // qDebug() << "Addresses in tx_out" << addresses;
                QVariant addr_id;
                if (addresses.count() > 0) {
                    addr_id = db.addBitcoinAddress(std::make_tuple(addresses[0].toString()));
                } else {
                    addr_id = db.addBitcoinAddress(std::make_tuple("UNDEFINED"));
                }
                // qDebug() << "Greated BITCOINADDRES record ID" << addr_id;
                if(!addr_id.isValid()) {
                    qDebug() << "Write in DB record of Address has ERROR!";
                    return EXIT_FAILURE;
                }
                // qDebug() << "nValue" << txvout[j].toObject().value("value");
                QVariant txvout_id = db.addTxOut(std::make_tuple(txvout[j].toObject().value("value").toDouble() * COIN,
                                                                 txvout[j].toObject().value("scriptPubKey").toObject().value("hex").toString(),
                                                                 txvout[j].toObject().value("n").toInt(),
                                                                 addr_id.toInt()));
                if(!txvout_id.isValid()) {
                    qDebug() << "Write in DB record of vin has ERROR!";
                    return EXIT_FAILURE;
                }
                if (!db.addTxVout(std::make_tuple(tx_id.toInt(), txvout_id.toInt()))){
                    qDebug() << "Write in DB record of tx_txout has ERROR!";
                    return EXIT_FAILURE;
                }

            }
        }
    }

    qDebug() <<  "\n FINISH!";
    return EXIT_SUCCESS;
}
