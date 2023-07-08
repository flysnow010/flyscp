#ifndef RSA_H
#define RSA_H

#include "config.h"
#include <QString>

class RSA_EXPORT Rsa
{
public:
    Rsa();

    static bool MakeKeys(QString & privateKey, QString & publicKey, int keyLength = 1024);
    static QString Encrypt(QString const& publicKey, QString const& text);
    static QString Decrypt(QString const& privateKey, QString const& cryptText);
};

#endif // RSA_H
