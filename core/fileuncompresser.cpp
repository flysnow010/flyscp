#include "fileuncompresser.h"
#include "winshell.h"
#include "util/utils.h"

#include <QStringList>
#include <QProcess>
#include <QDebug>

QString UncompressParam::overwriteMode() const
{
    if(isWithPath)
        return QString();

    if(mode == OverWrite)
        return QString(" -aoa");
    else if(mode == Skip)
        return QString(" -aos");
    else if(mode == AutoRename)
        return QString(" -aot");
    return QString();
}

FileUncompresser::FileUncompresser(QObject *parent)
    : QObject(parent)
    , process(new QProcess(this))
    , mode(Uncompress)
    , isEncrypted_(false)
    , isListStart_(false)
{
    connect(process, &QProcess::readyReadStandardOutput, this, [=](){
        while(process->canReadLine())
        {
            QString text = QString::fromUtf8(process->readLine()).remove("\r\n");
            if(!text.isEmpty())
            {
                if(mode == Uncompress)
                    emit progress(text);
                else if(mode == CheckEncrypt)
                {
                    if(text.contains("Encrypted = +"))
                        isEncrypted_ = true;
                }
                else if(mode == List)
                {
                    /*
                       Date      Time    Attr         Size   Compressed  Name
                    ------------------- ----- ------------ ------------  ------------------------
                    2022-04-26 10:32:54 ....A     79463864     78946166  VSCodeUserSetup-x64-1.66.2.exe
                    ------------------- ----- ------------ ------------  ------------------------
                    2022-04-26 10:32:54           79463864     78946166  1 files
                    */
                    if(text.startsWith("-------------------"))                    
                       isListStart_ = !isListStart_;
                    else if(isListStart_)
                        fileInfos << text;
                }
            }
        }
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if(mode != Uncompress)
            return;
        if(exitStatus != QProcess::ExitStatus::NormalExit)
            onError("Program is crash exit");
        else {
            if(exitCode != 0)
                onError(errorToText(exitCode));
            else {
                if(++currentIndex == argsList.size())
                    emit finished();
                else {
                    process->setArguments(nextArgs());
                    process->start();
                }
            }
        }
    });
}

bool FileUncompresser::uncompress(QStringList const& fileNames,
                UncompressParam const& param,
                QString const& targetFilePath)
{
    mode = Uncompress;
    process->setProgram(Utils::compressApp());
    argsList.clear();
    foreach(auto const& fileName, fileNames)
    {
        QStringList args;
        args << (param.isWithPath ? "x" : "e")
             << fileName;

        if(!param.isCreateDir)
            args << "-o" + targetFilePath;
        else
        {
            QDir dir(targetFilePath);
            QString pathName = QFileInfo(fileName).baseName();
            dir.mkdir(pathName);
            args <<  "-o" + dir.filePath(pathName);
        }
        args << param.filter;
        if(!param.password.isEmpty())
            args << "-p" + param.password;
        if(!param.isWithPath)
            args <<  param.overwriteMode() << "-y";
        argsList << args;
    }
    currentIndex = 0;
    process->setArguments(nextArgs());
    process->start();
    return true;
}

bool FileUncompresser::isEncrypted(QString const& fileName)
{
    QStringList args;
    args << "l"  << "-slt" << fileName;
    argsList.clear();
    argsList << args;
    mode = CheckEncrypt;
    currentIndex = 0;
    process->setProgram(Utils::compressApp());
    process->setArguments(nextArgs());
    process->start();
    process->waitForFinished();
    return isEncrypted_;
}

QStringList FileUncompresser::listFileInfo(QString const& fileName)
{
    QStringList args;
    args << "l"  << fileName;
    argsList.clear();
    argsList << args;
    mode = List;
    currentIndex = 0;
    process->setProgram(Utils::compressApp());
    process->setArguments(nextArgs());
    fileInfos.clear();
    process->start();
    process->waitForFinished();
    return fileInfos;
}

void FileUncompresser::cancel()
{
    if (process->state() == QProcess::NotRunning)
    {
        emit finished();
        return;
    }
    process->disconnect();
    process->kill();
    process->waitForFinished(1000);
    onError("User stopped the process");
}

QStringList FileUncompresser::nextArgs() const
{
    return  argsList.at(currentIndex);
}

void FileUncompresser::onError(QString const& errorText)
{
    emit error(errorText);
    emit finished();
}

QString FileUncompresser::errorToText(int errorCode) const
{
    if(errorCode == 1)
        return QString("Warning");
    else if(errorCode == 2)
        return QString("Fatal error");
    else if(errorCode == 7)
        return QString("Command line error");
    else if(errorCode == 8)
        return QString("Not enough memory for operation");
    else if(errorCode == 255)
        return QString("User stopped the process");
    return QString();
}

bool FileUncompresser::isCompressFiles(QStringList const& fileNames, QString &unCompressfileName)
{
    static QStringList suffixs = QStringList()
            << "7z"                                //7z
            << "bz2" << "bzip2" << "tbz2" << "tbz" // BZIP2
            << "gz" << "gzip" << "tgz"             //GZIP
            << "tar"                               //TAR
            << "wim" << "swm" << "esd"             //WIM
            << "xz" << "txz"                       //XZ
            << "zip" << "zipx" << "jar" << "xpi"
            << "odt" << "ods" << "docx" << "xlsx"
            << "epub"                              //ZIP
            << "apm"                               //APM
            << "ar" << "a" << "deb" << "lib"       //AR
            << "arj"                               //ARJ
            << "cab"                               //CAB
            << "chm" << "chw" << "chi" << "chq"    //CHM
            << "msi" << "msp" << "doc" << "xls"
            << "ppt"                               //COMPOUND
            << "cpio"                              //CPIO
            << "cramfs"                            //CramFS
            << "dmg"                               //DMG
            << "ext" << "ext2" << "ext3" << "ext4"
            << "img"                               //Ext
            << "fat" << "img"                      //FAT
            << "hfs" << "hfsx"                     //HFS
            << "hxs" << "hxi" << "hxr" << "hxq"
            << "hxw" << "lit"                      //HXS
            << "ihex"                              //iHEX
            << "iso" << "img"                      //ISO
            << "lzh" << "lha"                      //LZH
            << "lzma"                              //LZMA
            << "mbr"                               //MBR
            << "mslz"                              //MsLZ
            << "mub"                               //Mub
            << "nsis"                              //NSIS
            << "ntfs" << "img"                     //NTFS
            << "rar" << "r00"                      //RAR
            << "rpm"                               //RPM
            << "ppmd"                              //PPMD
            << "qcow" << "qcow2" << "qcow2c"       //QCOW2
            << "squashfs"                          //SquashFS
            << "udf" << "iso" << "img"             //UDF
            << "scap"                              //UEFIc
            << "uefif"                             //UEFIs
            << "vdi"                               //VDI
            << "vhd"                               //VHD
            << "vmdk"                              //VMDK
            << "xar" << "pkg"                      //XAR
            << "z" << "taz";                       //Z
    foreach(auto const& fileName, fileNames)
    {
        QString suffix = QFileInfo(fileName).suffix().toLower();
        if(!suffixs.contains(suffix))
        {
            bool isOK = false;
            suffix.toUInt(&isOK);
            if(!isOK)
                unCompressfileName = fileName;
            return isOK;
        }
    }
    return true;
}
