#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <QtNetwork>
#include <QDebug>
#include <QAuthenticator>
#include "qjsonrpchttpclient.h"
#include<QJsonDocument>

/*

 This file defines Slassi for working with a core Bitcoin wallet using RPC

*/


class HttpClient : public QJsonRpcHttpClient
{
    Q_OBJECT
public:
    HttpClient(const QString &endpoint, QObject *parent = 0)
        : QJsonRpcHttpClient(endpoint, parent){
        // defaults added for my local test server
        m_username = "plc";
        m_password = "plc";
    }

    void setUsername(const QString &username);
    void setPassword(const QString &password);
    QVariant getLastBlockNumber();
    QVariant getBlockHash(int blocknumber);
    QVariant getBlock(QString hash);

private Q_SLOTS:
    virtual void handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator * authenticator)
    {
        Q_UNUSED(reply)
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
    }

private:
    QString m_username;
    QString m_password;

};

#endif // HTTPCLIENT_H
