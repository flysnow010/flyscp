#include "remotefilemanager.h"

RemoteFileManager::RemoteFileManager(QObject *parent)
    : QObject(parent)
    , signal_(false)
{
}

void RemoteFileManager::cancel()
{
    doSignal();
}
