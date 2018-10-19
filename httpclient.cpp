#include "httpclient.h"

//******************************************************************************************
//******************************************************************************************
void HttpClient::setUsername(const QString &username) {
    m_username = username;
}

//******************************************************************************************
//******************************************************************************************
void HttpClient::setPassword(const QString &password) {
    m_password = password;
}

//******************************************************************************************
//******************************************************************************************
QVariant HttpClient::getLastBlockNumber(){
    QJsonArray params = {};
    QJsonDocument doc;
    QJsonParseError docError;

    QJsonRpcMessage message = QJsonRpcMessage::createRequest("getblockcount",params);
    QJsonRpcMessage response = this->sendMessageBlocking(message);
    if (response.type() == QJsonRpcMessage::Error) {
        return QVariant();
    }
    doc = QJsonDocument::fromJson(response.toJson(), &docError);
    if(!(docError.errorString().toInt() == QJsonParseError::NoError)) {
        return QVariant();
    }
    QJsonValue count = doc.object().value("result");
    return QVariant(count.toVariant());
}

//******************************************************************************************
//******************************************************************************************
QVariant HttpClient::getBlockHash(int blocknumber){
    QJsonArray params = {};
    QJsonDocument doc;
    QJsonParseError docError;

    params.append(blocknumber);
    QJsonRpcMessage message = QJsonRpcMessage::createRequest("getblockhash",params);
    QJsonRpcMessage response = this->sendMessageBlocking(message);
    if (response.type() == QJsonRpcMessage::Error) {
        return QVariant();
    }
    doc = QJsonDocument::fromJson(response.toJson(), &docError);
    if(!(docError.errorString().toInt() == QJsonParseError::NoError)) {
        return QVariant();
    }
    QJsonValue count = doc.object().value("result");
    return QVariant(count.toVariant());
}

//******************************************************************************************
//******************************************************************************************
QVariant HttpClient::getBlock(QString hash){
    QJsonArray params = {};
    QJsonDocument doc;
    QJsonParseError docError;

    params.append(hash);
    params.append(2);
    QJsonRpcMessage message = QJsonRpcMessage::createRequest("getblock",params);
    QJsonRpcMessage response = this->sendMessageBlocking(message);
    if (response.type() == QJsonRpcMessage::Error) {
        return QVariant();
    }
    doc = QJsonDocument::fromJson(response.toJson(), &docError);
    if(!(docError.errorString().toInt() == QJsonParseError::NoError)) {
        return QVariant();
    }
    //qDebug() << doc.toVariant();
    return doc.toVariant();
}
