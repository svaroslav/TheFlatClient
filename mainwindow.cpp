#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    if(initLogFile())
    {
        log(LogType::Info, tr("Application started"));
        ui->setupUi(this);
        ui->progressLoading->setValue(0);
        connect(&downloadManager,SIGNAL(finished(QNetworkReply*)),SLOT(downloadFinished(QNetworkReply*)));
        connect(&updateCheckManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateCheckFinished(QNetworkReply*)));
        if(checkAppFiles())
        {
            this->setWindowIcon(QIcon("Sources/System/grass_block_icon.png"));
            log(LogType::Info, tr("File initialization done"));
            if(loadSettingsFromFile())
            {
                log(LogType::Info, tr("Settings reading done"));
                if(settings.debugMode)
                {
                    showDebugConsoleDock();
                }
                if(settings.windowMaximized == true)
                {
                    log(LogType::Info, tr("Setting main window to maximized state"));
                    this->setWindowState(Qt::WindowMaximized);
                }
                if(settings.checkForUpdates == true)
                {
                    checkForUpdates();
                }
                setupGameView();
                setupSound();
                showMainMenu();
            }
            else
            {
                QMessageBox::warning(this, this->windowTitle() + "Error", tr("Failed to load application settings! See log for more details."));
            }
        }
        else
        {
            if(fileDownloadIsRunning)
            {
                QMessageBox::warning(this, this->windowTitle() + "Error", tr("Failed to initialize application system files! App trying to download these files, please wait a couple seconds to finish downloading and try start app again. See log for more details."));
            }
            else
            {
                QMessageBox::warning(this, this->windowTitle() + "Error", tr("Failed to initialize application system files! See log for more details."));
            }
            exit(2);
        }
    }
    else
    {
        QMessageBox::warning(this, this->windowTitle() + "Error", tr("Failed to initialize application system files! Could not create log file!"));
        exit(2);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::initLogFile()
{
    QDir fileFolder(appDataFilePath);
    if(!fileFolder.exists())
    {
        if(!fileFolder.mkdir(appDataFilePath))
        {
            qDebug() << "Failed to initialize app folder";
        }
    }

    logFile = new QFile(appDataFilePath + "/log.txt");
    if(!logFile->exists())
    {
        if(logFile->open(QIODevice::WriteOnly))
        {
            logFile->close();
        }
        else
        {
            qDebug() << "Failed to initialize log file";
        }
    }
    else
    {
        QDir dir;
        QFile tmp(appDataFilePath + "/log.old.txt");
        if(tmp.exists())//pokud již existuje old soubor
        {
            if(!dir.remove(appDataFilePath + "/log.old.txt"))
            {
                qDebug() << "Failed to remove old log file";
            }
        }
        if(!dir.rename(appDataFilePath + "/log.txt", appDataFilePath + "/log.old.txt"))
        {
            qDebug() << "Failed to rename log to log.old";
        }
    }

    if(logFile->open(QIODevice::ReadOnly | QFile::Append))
    {
        qDebug() << "Log file initialized";
        return true;//pokud probehne bez chyby vrati true
    }
    else
    {
        return false;
    }
}

bool MainWindow::checkAppFiles()
{
    bool err = false;
    log(LogType::Info, tr("Initializing user settings"));
    ui->labelLoading->setText(tr("Initializing user settings"));

    QDir settingsFolder(QString(appDataFilePath + "/Settings"));
    if(!settingsFolder.exists())
    {
        log(LogType::Warning, tr("'Settings' folder not found, creating"));
        if(!settingsFolder.mkdir(appDataFilePath + "/Settings"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'Settings' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    log(LogType::Info, tr("Initializing user saves"));
    ui->labelLoading->setText(tr("Initializing user saves"));

    QDir savesFolder(QString(appDataFilePath + "/Saves"));
    if(!savesFolder.exists())
    {
        log(LogType::Warning, tr("'Saves' folder not found, creating"));
        if(!savesFolder.mkdir(appDataFilePath + "/Saves"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'Saves' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    log(LogType::Info, tr("Initializing application files"));
    ui->labelLoading->setText(tr("Initializing application files"));

    QDir sourcesFolder("Sources");
    if(!sourcesFolder.exists())
    {
        log(LogType::Warning, tr("'Sources' folder not found, creating"));
        if(!sourcesFolder.mkdir(QDir::currentPath() + "/Sources"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'Sources' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    QDir systemFolder("Sources/System");
    if(!systemFolder.exists())
    {
        log(LogType::Warning, tr("'System' folder not found, creating"));
        if(!systemFolder.mkdir(QDir::currentPath() + "/Sources/System"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'System' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    if(!err)
    {
        QFile file;
        QString path;
        for(int i = 0; i < systemFileList.length(); ++i)
        {
            path = systemFolder.path() + "/" + systemFileList.at(i);
            file.setFileName(path);
            if(!file.exists())
            {
                log(LogType::Error, tr("Failed to load file ") + path);
                downloadAppFile(QString("System/" + systemFileList.at(i)));
            }
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    QDir texturesFolder("Sources/Textures");
    if(!texturesFolder.exists())
    {
        log(LogType::Warning, tr("'Textures' folder not found, creating"));
        if(!texturesFolder.mkdir(QDir::currentPath() + "/Sources/Textures"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'Textures' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    if(!err)
    {
        QFile file;
        QString path;
        for(int i = 0; i < texturesFileList.length(); ++i)
        {
            path = texturesFolder.path() + "/" + texturesFileList.at(i);
            file.setFileName(path);
            if(!file.exists())
            {
                log(LogType::Error, tr("Failed to load file ") + path);
                downloadAppFile(QString("Textures/" + texturesFileList.at(i)));
            }
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    QDir soundsFolder("Sources/Sounds");
    if(!soundsFolder.exists())
    {
        log(LogType::Error, tr("'Sounds' folder not found, creating"));
        if(!soundsFolder.mkdir(QDir::currentPath() + "/Sources/Sounds"))
        {
            err = true;
            log(LogType::Error, tr("Failed to create 'Sounds' folder"));
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    if(!err)
    {
        QFile file;
        QString path;
        for(int i = 0; i < soundsFileList.length(); ++i)
        {
            path = soundsFolder.path() + "/" + soundsFileList.at(i);
            file.setFileName(path);
            if(!file.exists())
            {
                log(LogType::Error, tr("Failed to load file ") + path);
                downloadAppFile(QString("Sounds/" + soundsFileList.at(i)));
            }
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    return !err;//pokud proběhne bez chyby, vrátí true
}

void MainWindow::downloadAppFile(QString fileName)
{
    fileDownloadIsRunning = true;
    downloadFileCount++;
    log(LogType::Info, tr("Downloading file: ") + fileName);
    QString url = fileServerUrl + fileName;
    downloadRequest.setUrl(QUrl(url));
    downloadManager.get(downloadRequest);
}

bool MainWindow::loadSettingsFromFile()
{
    bool err = false;

    log(LogType::Info, tr("Loading common settings"));
    ui->labelLoading->setText(tr("Loading common settings"));

    QFile commonFile(QString(appDataFilePath + "/Settings/common.flat"));
    if(!commonFile.exists())
    {
        if(!commonFile.open(QIODevice::WriteOnly))
        {
            err = true;
            log(LogType::Error, tr("Failed to create common configuration file!"));
        }
        else
        {
            QJsonDocument document;
            QJsonObject obj;
            QJsonValue checkForUpdates = QJsonValue(true);
            QJsonValue latestServerAddress = QJsonValue(QHostAddress(QHostAddress::Broadcast).toString());
            QJsonValue latestServerPort = QJsonValue(QString::number(-1));
            QJsonValue debugMode = QJsonValue(false);

            obj["checkForUpdates"] = checkForUpdates;
            obj["latestServerAddress"] = latestServerAddress;
            obj["latestServerPort"] = latestServerPort;
            obj["debugMode"] = debugMode;
            document.setObject(obj);
            QString defaultSettingsString = QString(document.toJson());

            QDataStream out(&commonFile);
            out.setVersion(QDataStream::Qt_5_0);
            out << defaultSettingsString;
            commonFile.close();
        }
    }

    if(!commonFile.open(QIODevice::ReadOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to open common configuration file!"));
    }
    else
    {
        log(LogType::Info, tr("Parsing common settings file"));
        ui->labelLoading->setText(tr("Parsing common settings file"));

        QString data;
        QDataStream in(&commonFile);
        in.setVersion(QDataStream::Qt_5_0);
        in.startTransaction();
        in >> data;
        qDebug() << data;

        QJsonDocument document = QJsonDocument::fromJson(data.toLatin1());
        QJsonObject obj = document.object();
        if(obj.contains("checkForUpdates") && obj["checkForUpdates"].isBool())
        {
            settings.checkForUpdates = obj["checkForUpdates"].toBool();
        }
        if(obj.contains("latestServerAddress") && obj["latestServerAddress"].isString())
        {
            settings.latestServerAddress = QHostAddress(obj["latestServerAddress"].toString());
        }
        if(obj.contains("latestServerPort") && obj["latestServerPort"].isString())
        {
            settings.latestServerPort = obj["latestServerPort"].toString().toInt();
        }
        if(obj.contains("debugMode") && obj["debugMode"].isBool())
        {
            settings.debugMode = obj["debugMode"].toBool();
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    log(LogType::Info, tr("Loading user interface settings"));
    ui->labelLoading->setText(tr("Loading user interface settings"));

    QFile userInterfaceFile(QString(appDataFilePath + "/Settings/userInterface.flat"));
    if(!userInterfaceFile.exists())
    {
        if(!userInterfaceFile.open(QIODevice::WriteOnly))
        {
            err = true;
            log(LogType::Error, tr("Failed to create user interface configuration file!"));
        }
        else
        {
            QJsonDocument document;
            QJsonObject obj;
            QJsonValue language = QJsonValue("cs_CZ");
            QJsonValue username = QJsonValue("Player one");
            QJsonValue windowFullSreen = QJsonValue(false);
            QJsonValue windowMaximized = QJsonValue(true);

            obj["language"] = language;
            obj["username"] = username;
            obj["windowFullScreen"] = windowFullSreen;
            obj["windowMaximized"] = windowMaximized;
            document.setObject(obj);
            QString defaultSettingsString = QString(document.toJson());

            QDataStream out(&userInterfaceFile);
            out.setVersion(QDataStream::Qt_5_0);
            out << defaultSettingsString;
            userInterfaceFile.close();
        }
    }

    if(!userInterfaceFile.open(QIODevice::ReadOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to open user interface configuration file!"));
    }
    else
    {
        log(LogType::Info, tr("Parsing user interface settings file"));
        ui->labelLoading->setText(tr("Parsing user interface settings file"));

        QString data;
        QDataStream in(&userInterfaceFile);
        in.setVersion(QDataStream::Qt_5_0);
        in.startTransaction();
        in >> data;
        qDebug() << data;

        QJsonDocument document = QJsonDocument::fromJson(data.toLatin1());
        QJsonObject obj = document.object();
        if(obj.contains("language") && obj["language"].isString())
        {
            settings.language = obj["language"].toString();
        }
        if(obj.contains("username") && obj["username"].isString())
        {
            settings.username = obj["username"].toString();
        }
        if(obj.contains("windowFullScreen") && obj["windowFullScreen"].isBool())
        {
            settings.windowFullScreen = obj["windowFullScreen"].toBool();
        }
        if(obj.contains("windowMaximized") && obj["windowMaximized"].isBool())
        {
            settings.windowMaximized = obj["windowMaximized"].toBool();
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    log(LogType::Info, tr("Loading sounds settings"));
    ui->labelLoading->setText(tr("Loading sounds settings"));

    QFile soundsFile(QString(appDataFilePath + "/Settings/sounds.flat"));
    if(!soundsFile.exists())
    {
        if(!soundsFile.open(QIODevice::WriteOnly))
        {
            err = true;
            log(LogType::Error, tr("Failed to create sounds configuration file!"));
        }
        else
        {
            QJsonDocument document;
            QJsonObject obj;
            QJsonValue soundEnabled = QJsonValue(true);
            QJsonValue soundVolume = QJsonValue(QString::number(50));

            obj["soundEnabled"] = soundEnabled;
            obj["soundVolume"] = soundVolume;
            document.setObject(obj);
            QString defaultSettingsString = QString(document.toJson());

            QDataStream out(&soundsFile);
            out.setVersion(QDataStream::Qt_5_0);
            out << defaultSettingsString;
            soundsFile.close();
        }
    }

    if(!soundsFile.open(QIODevice::ReadOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to open sounds configuration file!"));
    }
    else
    {
        log(LogType::Info, tr("Parsing sounds settings file"));
        ui->labelLoading->setText(tr("Parsing sounds settings file"));

        QString data;
        QDataStream in(&soundsFile);
        in.setVersion(QDataStream::Qt_5_0);
        in.startTransaction();
        in >> data;
        qDebug() << data;

        QJsonDocument document = QJsonDocument::fromJson(data.toLatin1());
        QJsonObject obj = document.object();
        if(obj.contains("soundEnabled") && obj["soundEnabled"].isBool())
        {
            settings.soundEnabled = obj["soundEnabled"].toBool();
        }
        if(obj.contains("soundVolume") && obj["soundVolume"].isString())
        {
            settings.soundVolume = obj["soundVolume"].toString().toInt();
            qDebug() << "settings loaded sound volume" << obj["soundVolume"].toString().toInt();
        }
    }

    ui->progressLoading->setValue(ui->progressLoading->value() + 1);

    return !err;
}

void MainWindow::setupSound()
{
    log(LogType::Info, tr("Starting audio"));
    backgroundAudioPlayer = new QMediaPlayer;
    backgroundAudioPlayer->setVolume(0);
    backgroundAudioPlaylist = new QMediaPlaylist;
    if(settings.soundEnabled)
    {
        if(backgroundAudioLoadPlaylist())
        {
            log(LogType::Info, tr("Done creating background audio playlist"));
            backgroundAudioPlayer->setVolume(settings.soundVolume);
            backgroundAudioPlayer->setPlaylist(backgroundAudioPlaylist);
            connect(backgroundAudioPlaylist, &QMediaPlaylist::currentIndexChanged, this, &MainWindow::onBackgroundAudioPlaylistPositionChanged);
            backgroundAudioPlayer->play();
        }
        else
        {
            log(LogType::Warning, tr("Audio folder is empty, could not create background audio playlist"));
        }
    }
}

bool MainWindow::backgroundAudioLoadPlaylist()
{
    log(LogType::Info, tr("Loading background audio playlist"));
    bool contain = false;
    QDir soundsDir(QDir::currentPath() + "/Sources/Sounds");
    QFileInfoList fileList = soundsDir.entryInfoList(QDir::Files);
    for(int i = 0; i < fileList.length(); ++i)
    {
        backgroundAudioPlaylist->addMedia(QUrl::fromLocalFile(fileList.at(i).filePath()));
        contain = true;
    }
    return contain;
}

void MainWindow::serverNewConnection()
{
    log(LogType::Info, tr("Server requested new connection"));
    QTcpSocket *tmpSocket = server->nextPendingConnection();
    connect(tmpSocket, &QTcpSocket::readyRead, this, &MainWindow::serverReadyRead);
    connect(tmpSocket, &QTcpSocket::disconnected, this, &MainWindow::serverDisconnected);
    Client tmpClient;
    tmpClient.pointer = tmpSocket;
    tmpClient.state = ClientState::Connecting;
    waitingClientList.append(tmpClient);
}

void MainWindow::serverReadyRead()
{
    QTcpSocket *clientPointer = static_cast<QTcpSocket *>(sender());
    log(LogType::Info, tr("Server has incoming data from ") + clientPointer->peerAddress().toString() + ":" + QString::number(clientPointer->peerPort()));
    QByteArray data;
    QDataStream read(clientPointer);
    read.setVersion(QDataStream::Qt_5_0);
    read.startTransaction();
    read >> data;
    read.abortTransaction();

    processReadData(data, clientPointer);
}

void MainWindow::processReadData(QByteArray data, QTcpSocket *clientPointer)
{
    qDebug() << data;

    bool err = true;
    int clientIndex = -1;
    for(int i = 0; i < serverClientList.length(); ++i)
    {
        if(serverClientList.at(i).pointer == clientPointer)
        {
            clientIndex = i;
            err = false;
            break;
        }
    }
    if(err)
    {
        log(LogType::Error, tr("An error has occured while reading incoming data from client ") + clientPointer->peerAddress().toString() + ":" + QString::number(clientPointer->peerPort()));
    }
    else
    {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        if(obj.contains("application") && obj["application"].isString() && obj["application"] == "The Flat")
        {
            if(obj.contains("action") && obj["action"].isString())
            {

            }
            else
            {
                qDebug() << "Incoming data is not containing action data";
            }
        }
        else
        {
            qDebug() << "Incoming data is not from The Flat";
        }
    }
}

void MainWindow::serverDisconnected()
{
    QTcpSocket *client = static_cast<QTcpSocket *>(sender());
    log(LogType::Info, tr("Client ") + client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + " disconnected");
}

bool MainWindow::startServer()
{
    server = new QTcpServer;
    connect(server, &QTcpServer::newConnection, this, &MainWindow::serverNewConnection);
    if(server->listen(runningServerInfo.address, runningServerInfo.port))
    {
        log(LogType::Info, tr("Server started on address ") + runningServerInfo.address.toString() + tr(" and port ") + QString::number(runningServerInfo.port));
        return true;
    }
    else
    {
        log(LogType::Warning, tr("Failed to start server!"));
        return false;
    }
}

void MainWindow::disconnectAllUsers()
{
    log(LogType::Info, tr("Disconnecting all clients"));
    int count = serverClientList.length();
    for(int i = 0; i < count; ++i)
    {
        serverClientList.at(i).pointer->disconnectFromHost();
        log(LogType::Info, tr("Client ") + QString::number(i + 1) + tr(" of ") + QString::number(count) + tr(" has been disconnected"));
    }
}

void MainWindow::stopServer()
{
    log(LogType::Info, tr("Stopping server"));
    if(gameIsActive == false)
    {
        stopBroadcastInfo();
    }
    disconnectAllUsers();
    server->close();
    serverIsActive = false;
    log(LogType::Info, tr("Server stopped"));
}

void MainWindow::promoteServer()
{
    log(LogType::Info, tr("Requesting for my server promotion - '") + runningServerInfo.name + QString("' with maximum of ") + QString::number(runningServerInfo.numberOfClients) + QString(" clients"));
}

void MainWindow::sendBroadcastInfo()
{
    broadcastTimer->start();
    runningServerInfo.quickDescribtion = "nejaky usele pospi";
    QByteArray data = QString(QString("{\"application\":\"The Flat\",\"action\":\"Server is available\",\"server\":{\"name\":\"") + runningServerInfo.name + QString("\",\"quickDescribtion\":\"") + runningServerInfo.quickDescribtion + QString("\",\"address\":\"") + runningServerInfo.address.toString() + QString("\",\"port\":\"") + QString::number(runningServerInfo.port) + QString("\"}}")).toLatin1();
    broadcastSocket->writeDatagram(data, QHostAddress::Broadcast, 5115);
}

void MainWindow::startBroadcastInfo()
{
    int interval = 5000;
    broadcastSocket = new QUdpSocket(this);
    broadcastTimer = new QTimer;
    broadcastTimer->setInterval(interval);
    connect(broadcastTimer, &QTimer::timeout, this, &MainWindow::sendBroadcastInfo);
    broadcastTimer->start();
    log(LogType::Info, tr("Broadcast info started for every ") + QString::number((double)interval / 1000) + " seconds");
    sendBroadcastInfo();//první odeslání, aby se nečekalo
}

void MainWindow::stopBroadcastInfo()
{
    broadcastTimer->stop();
    log(LogType::Info, tr("Broadcast info stopped"));
}

bool MainWindow::serverCheckClientsReady()
{
    for(int i = 0; i < serverClientList.length(); ++i)
    {
        if(serverClientList.at(i).ready == false)
        {
            log(LogType::Info, tr("Checking ready clients - some clients are not ready"));
            return false;
        }
    }
    log(LogType::Info, tr("Checking ready clients - all clients are ready"));
    return true;
}

void MainWindow::serverStartingGameTimeout()
{
    int tmp = 5 - serverStartingTimeoutInt;
    serverStartingTimeoutInt++;
    if(serverStartingTimeoutInt >= 5)
    {
        serverStartingTimeoutInt = 0;
    }
    log(LogType::Game, tr("Starting game ... ") + QString::number(tmp));
}

void MainWindow::serverStartGame()
{
    log(LogType::Game, tr("Game started"));
}

void MainWindow::serverSendToAllClients(QByteArray data)
{
    log(LogType::Game, tr("Server sending data to all clients: ") + QString::fromLatin1(data));

    for(int i = 0; i < serverClientList.length(); ++i)
    {
        QByteArray tmp = data;
        QDataStream write(serverClientList.at(i).pointer);
        write.setVersion(QDataStream::Qt_5_0);
        write << tmp;
    }
}

MainWindow::ServerInfo MainWindow::getServerInfo()
{
    if(serverIsActive)
    {
        return runningServerInfo;
    }
    else
    {
        return connectedServerInfo;
    }
}

bool MainWindow::connectToServer(QHostAddress address, int port)
{
    log(LogType::Info, tr("Connecting to server ") + address.toString() + ":" + QString::number(port));

    clientSocket = new QTcpSocket;

    clientSocket->connectToHost(address, port);
    connect(clientSocket, &QTcpSocket::connected, this, &MainWindow::socketConnectedToServer);
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::clientReadyRead);
    QTimer *connectingTimeout = new QTimer;
    connectingTimeout->setInterval(10000);//10 sekund
    connectingTimeout->start();
    connect(connectingTimeout, &QTimer::timeout, this, &MainWindow::connectingToServerTimeout);
    while(true)
    {
        if(connectedToServer == true)
        {
            return true;
        }
        if(connectingToServerCanceled)
        {
            connectingToServerCanceled = false;
            return false;
        }
    }
}

void MainWindow::socketConnectedToServer()
{
    log(LogType::Info, tr("Connected to server"));
    connectedToServer = true;
    clientConnectionState = ClientState::Connecting;


}

void MainWindow::connectingToServerTimeout()
{
    connectingToServerCanceled = true;
}

void MainWindow::listenForBroadcast()
{
    broadcastClientSocket = new QUdpSocket(this);
    broadcastClientSocket->bind(5115, QUdpSocket::ShareAddress);
    connect(broadcastClientSocket, &QUdpSocket::readyRead, this, &MainWindow::udpSocketPendingDatagram);
    log(LogType::Info, tr("Listening for broadcasted lan server info"));
}

void MainWindow::stopListeningForBroadcast()
{
    broadcastClientSocket->close();
    delete broadcastClientSocket;
    log(LogType::Info, tr("Listening for broadcasted stopped"));
}

void MainWindow::disconnectFromServer()
{
    clientSocket->disconnectFromHost();
    log(LogType::Info, tr("Disconnected from server"));
}

void MainWindow::clientReadyRead()
{
    log(LogType::Info, tr("Received data from server ") + clientSocket->peerAddress().toString() + ":" + QString::number(clientSocket->peerPort()));
    QByteArray data;
    QDataStream read(clientSocket);
    read.setVersion(QDataStream::Qt_5_0);
    read.startTransaction();
    read >> data;
    read.abortTransaction();

    clientProcessReadData(data);
}

void MainWindow::clientProcessReadData(QByteArray data)
{
    qDebug() << data;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    if(obj.contains("application") && obj["application"].isString() && obj["application"] == "The Flat")
    {
        if(obj.contains("action") && obj["action"].isString())
        {

        }
        else
        {
            qDebug() << "Incoming data is not containing action data";
        }
    }
    else
    {
        qDebug() << "Incoming data is not from The Flat";
    }
}

void MainWindow::udpSocketPendingDatagram()
{
    QByteArray data;
    while (broadcastClientSocket->hasPendingDatagrams())
    {
        data.resize(int(broadcastClientSocket->pendingDatagramSize()));
        broadcastClientSocket->readDatagram(data.data(), data.size());
        qDebug() << "Received broadcast:" << data;
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        if(obj.contains("application") && obj["application"].isString() && obj["application"] == "The Flat")
        {
            if(obj.contains("action") && obj["action"].isString() && obj["action"] == "Server is available")
            {
                bool err = false;
                ServerInfo tmp;
                if(obj.contains("name") && obj["name"].isString())
                {
                    tmp.name = obj["name"].toString();
                }
                else
                {
                    err = true;
                }
                if(obj.contains("address") && obj["address"].isString())
                {
                    tmp.address = QHostAddress(obj["address"].toString());
                }
                else
                {
                    err = true;
                }
                if(obj.contains("quickDescribtion") && obj["quickDescribtion"].isString())
                {
                    tmp.quickDescribtion = obj["quickDescribtion"].toString();
                }
                else
                {
                    err = true;
                }
                if(obj.contains("port") && obj["port"].isString())
                {
                    tmp.port = obj["port"].toString().toInt();
                }
                else
                {
                    err = true;
                }
                if(obj.contains("quickDescribtion") && obj["quickDescribtion"].isString())
                {
                    tmp.quickDescribtion = obj["quickDescribtion"].toString();
                }
                else
                {
                    err = true;
                }

                if(!err)
                {
                    QListWidgetItem *item = new QListWidgetItem;
                    item->setIcon(tmp.icon);
                    item->setText(tmp.name);
                    item->setData(1, QVariant(QString(QHostAddress(tmp.address).toString() + ":" + QString::number(tmp.port))));
                    item->setToolTip(tmp.quickDescribtion);
                    listMultiplayerLANServers->addItem(item);
                    qDebug() << "hello";
//                    gameView->update();
                }
                else
                {
                    qDebug() << "nejaky err";
                }
            }
        }
    }
}

void MainWindow::setupGameView()
{
    log(LogType::Info, tr("Setting up graphics scene"));

    gameView = new GameView;
    this->setCentralWidget(gameView);

    gameScene = new QGraphicsScene;
    gameView->setScene(gameScene);

    if(settings.windowFullScreen == true)
    {
        log(LogType::Info, tr("Changing graphics view to fullscreen mode"));
        gameView->showFullScreen();
    }
}

void MainWindow::removeProxyItems()
{
    int length = proxyItemList.length();
    for(int i = 0; i < length; ++i)
    {
        gameScene->removeItem(proxyItemList.first());
        proxyItemList.removeFirst();
    }
}

void MainWindow::showMainMenu()
{
    log(LogType::Info, tr("Showing main menu"));
    this->setWindowTitle(QString("The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(mainMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeight = 50;
    int buttonsSpacing = 5;

    QLabel *labelMainMenuTitle = new QLabel(tr("The Flat"));
    labelMainMenuTitle->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeight * 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelMainMenuTitle->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelMainMenuTitle = gameScene->addWidget(labelMainMenuTitle);
    proxyLabelMainMenuTitle->setPos(0 - (proxyLabelMainMenuTitle->widget()->width() / 2), 0 - (1 * buttonsHeight) - proxyLabelMainMenuTitle->widget()->height());
    proxyItemList.append(proxyLabelMainMenuTitle);

    QPushButton *buttonSinglePlayer = new GraphicButton(tr("Singleplayer"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonSinglePlayer, &QPushButton::clicked, this, &MainWindow::onButtonSinglePlayerClicked);
    QGraphicsProxyWidget *proxyButtonSinglePlayer = gameScene->addWidget(buttonSinglePlayer);
    proxyButtonSinglePlayer->setPos(0 - (buttonsWidth / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonSinglePlayer);

    QPushButton *buttonMultiPlayer = new GraphicButton(tr("Multiplayer"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiPlayer, &QPushButton::clicked, this, &MainWindow::onButtonMultiPlayerClicked);
    QGraphicsProxyWidget *proxyButtonMultiPlayer = gameScene->addWidget(buttonMultiPlayer);
    proxyButtonMultiPlayer->setPos(0 - (buttonsWidth / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiPlayer);

    QPushButton *buttonSettings = new GraphicButton(tr("Settings"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonSettings, &QPushButton::clicked, this, &MainWindow::onButtonSettingsClicked);
    QGraphicsProxyWidget *proxyButtonSettings = gameScene->addWidget(buttonSettings);
    proxyButtonSettings->setPos(0 - (buttonsWidth / 2), 2 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonSettings);

    QPushButton *buttonDeveloperInfo = new GraphicButton(tr("Developers"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonDeveloperInfo, &QPushButton::clicked, this, &MainWindow::onButtonDeveloperInfoClicked);
    QGraphicsProxyWidget *proxyButtonDeveloperInfo = gameScene->addWidget(buttonDeveloperInfo);
    proxyButtonDeveloperInfo->setPos(0 - (buttonsWidth / 2), 3 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonDeveloperInfo);

    QPushButton *buttonQuit = new GraphicButton(tr("Quit"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonQuit, &QPushButton::clicked, this, &MainWindow::onButtonQuitClicked);
    QGraphicsProxyWidget *proxyButtonQuit = gameScene->addWidget(buttonQuit);
    proxyButtonQuit->setPos(0 - (buttonsWidth / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonQuit);
}

void MainWindow::showSettingsMenu()
{
    log(LogType::Info, tr("Showing settings menu"));
    this->setWindowTitle(QString(tr("Settings") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(settingsMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeight = 50;
    int buttonsSpacing = 5;

    QLabel *labelSettingsTitle = new QLabel(tr("Settings"));
    labelSettingsTitle->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeight) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelSettingsTitle->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelSettingsTitle = gameScene->addWidget(labelSettingsTitle);
    proxyLabelSettingsTitle->setPos(0 - (proxyLabelSettingsTitle->widget()->width() / 2), 0 - (1 * buttonsHeight) - proxyLabelSettingsTitle->widget()->height());
    proxyItemList.append(proxyLabelSettingsTitle);

    checkSettingsSoundEnabled = new QCheckBox(tr("Sounds enabled"));
    checkSettingsSoundEnabled->setStyleSheet(QString("font-weight: bold;"
                                                     "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                     "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                     "height: " + QString::number(buttonsHeight) + "px;"
                                                     "font-family:" + menuButtonFontFamily + ";"));
    checkSettingsSoundEnabled->setChecked(settings.soundEnabled);
    connect(checkSettingsSoundEnabled, &QCheckBox::toggled, this, &MainWindow::onCheckSettingsSoundEnabledToggled);
    checkSettingsSoundEnabled->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyCheckSettingsSoundEnabled = gameScene->addWidget(checkSettingsSoundEnabled);
    proxyCheckSettingsSoundEnabled->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyCheckSettingsSoundEnabled);

    checkSettingsWindowMaximized = new QCheckBox(tr("Window maximized"));
    checkSettingsWindowMaximized->setStyleSheet(QString("font-weight: bold;"
                                                     "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                     "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                     "height: " + QString::number(buttonsHeight) + "px;"
                                                     "font-family:" + menuButtonFontFamily + ";"));
    checkSettingsWindowMaximized->setChecked(settings.windowMaximized);
    connect(checkSettingsWindowMaximized, &QCheckBox::toggled, this, &MainWindow::onCheckSettingsWindowMaximizedToggled);
    checkSettingsWindowMaximized->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyCheckSettingsWindowMaximized = gameScene->addWidget(checkSettingsWindowMaximized);
    proxyCheckSettingsWindowMaximized->setPos(0 + (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyCheckSettingsWindowMaximized);

    checkSettingsCheckForUpdates = new QCheckBox(tr("Check for updates"));
    checkSettingsCheckForUpdates->setStyleSheet(QString("font-weight: bold;"
                                                     "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                     "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                     "height: " + QString::number(buttonsHeight) + "px;"
                                                     "font-family:" + menuButtonFontFamily + ";"));
    checkSettingsCheckForUpdates->setChecked(settings.checkForUpdates);
    connect(checkSettingsCheckForUpdates, &QCheckBox::toggled, this, &MainWindow::onCheckSettingsCheckForUpdatesToggled);
    checkSettingsCheckForUpdates->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyCheckSettingsCheckForUpdates = gameScene->addWidget(checkSettingsCheckForUpdates);
    proxyCheckSettingsCheckForUpdates->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyCheckSettingsCheckForUpdates);

    checkSettingsWindowFullScreen = new QCheckBox(tr("Window fullscreen"));
    checkSettingsWindowFullScreen->setStyleSheet(QString("font-weight: bold;"
                                                     "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                     "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                     "height: " + QString::number(buttonsHeight) + "px;"
                                                     "font-family:" + menuButtonFontFamily + ";"));
    checkSettingsWindowFullScreen->setChecked(settings.windowFullScreen);
    connect(checkSettingsWindowFullScreen, &QCheckBox::toggled, this, &MainWindow::onCheckSettingsWindowFullScreenToggled);
    checkSettingsWindowFullScreen->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyCheckSettingsWindowFullScreen = gameScene->addWidget(checkSettingsWindowFullScreen);
    proxyCheckSettingsWindowFullScreen->setPos(0 + (buttonsSpacing / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyCheckSettingsWindowFullScreen);

    QLabel *labelSettingsSoundVolume = new QLabel(tr("Sound volume"));
    labelSettingsSoundVolume->setStyleSheet(QString("font-weight: bold;"
                                                    "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                    "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                    "height: " + QString::number(buttonsHeight) + "px;"
                                                    "font-family:" + menuButtonFontFamily + ";"));
    labelSettingsSoundVolume->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelSettingsSoundVolume = gameScene->addWidget(labelSettingsSoundVolume);
    proxyLabelSettingsSoundVolume->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 2 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelSettingsSoundVolume);

    sliderSettingsSoundVolume = new QSlider(Qt::Horizontal);
    sliderSettingsSoundVolume->setMinimum(0);
    sliderSettingsSoundVolume->setMaximum(100);
    sliderSettingsSoundVolume->setValue(settings.soundVolume);
    connect(sliderSettingsSoundVolume, &QSlider::valueChanged, this, &MainWindow::onSliderSettingsSoundVolumeValueChanged);
    sliderSettingsSoundVolume->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxySliderSettingsSoundVolume = gameScene->addWidget(sliderSettingsSoundVolume);
    proxySliderSettingsSoundVolume->setPos(0 + (buttonsSpacing / 2), 2 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxySliderSettingsSoundVolume);

    QLabel *labelSettingsUsername = new QLabel(tr("Username"));
    labelSettingsUsername->setStyleSheet(QString("font-weight: bold;"
                                                    "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                    "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                    "height: " + QString::number(buttonsHeight) + "px;"
                                                    "font-family:" + menuButtonFontFamily + ";"));
    labelSettingsUsername->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelSettingsUsername = gameScene->addWidget(labelSettingsUsername);
    proxyLabelSettingsUsername->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 3 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelSettingsUsername);

    lineSettingsUsername = new QLineEdit;
    lineSettingsUsername->setPlaceholderText(tr("Your username here"));
    lineSettingsUsername->setText(settings.username);
    connect(lineSettingsUsername, &QLineEdit::returnPressed, this, &MainWindow::onLineSettingsUsernameReturnPressed);
    QGraphicsProxyWidget *proxyLineSettingsUsername = gameScene->addWidget(lineSettingsUsername);
    proxyLineSettingsUsername->setPos(0 + (buttonsSpacing / 2), 3 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLineSettingsUsername);

    QPushButton *buttonSettingsSave = new GraphicButton(tr("Save"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonSettingsSave, &QPushButton::clicked, this, &MainWindow::onButtonSettingsSaveClicked);
    proxyButtonSettingsSave = gameScene->addWidget(buttonSettingsSave);
    proxyButtonSettingsSave->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonSettingsSave);
    settingsMenuUpdateSaveButton();

    QPushButton *buttonSettingsBack = new GraphicButton(tr("Back"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonSettingsBack, &QPushButton::clicked, this, &MainWindow::onButtonSettingsBackClicked);
    QGraphicsProxyWidget *proxyButtonSettingsBack = gameScene->addWidget(buttonSettingsBack);
    proxyButtonSettingsBack->setPos(0 + (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonSettingsBack);
}

void MainWindow::settingsMenuUpdateSaveButton()
{
    if(someSettingsWasChanged)
    {
        proxyButtonSettingsSave->widget()->setDisabled(false);
        proxyButtonSettingsSave->widget()->setStyleSheet(proxyButtonSettingsSave->widget()->styleSheet().remove("color: rgb(" + QString::number(disabledMenuTextColor.red()) + ", " + QString::number(disabledMenuTextColor.green()) + ", " + QString::number(disabledMenuTextColor.blue()) + ");"));
    }
    else
    {
        proxyButtonSettingsSave->widget()->setDisabled(true);
        proxyButtonSettingsSave->widget()->setStyleSheet(proxyButtonSettingsSave->widget()->styleSheet() + "color: rgb(" + QString::number(disabledMenuTextColor.red()) + ", " + QString::number(disabledMenuTextColor.green()) + ", " + QString::number(disabledMenuTextColor.blue()) + ");");
    }
}

void MainWindow::saveSettings()
{
    log(LogType::Info, tr("Saving settings"));
    settings.checkForUpdates = checkSettingsCheckForUpdates->isChecked();
    settings.soundEnabled = checkSettingsSoundEnabled->isChecked();
    settings.windowMaximized = checkSettingsWindowMaximized->isChecked();
    settings.windowFullScreen = checkSettingsWindowFullScreen->isChecked();
    settings.soundVolume = sliderSettingsSoundVolume->value();
    settings.username = lineSettingsUsername->text();
    someSettingsWasChanged = false;
    settingsMenuUpdateSaveButton();
    if(saveSettingsToFile())
    {
        log(LogType::Info, tr("Settings succesfully saved"));
    }
    else
    {
        log(LogType::Warning, tr("Failed to write settings to file!"));
    }
}

bool MainWindow::saveSettingsToFile()
{
    bool err = false;
    QFile commonFile(QString(appDataFilePath + "/Settings/common.flat"));
    if(!commonFile.exists())
    {
        log(LogType::Error, tr("Common configuration file not found!"));
    }
    if(!commonFile.open(QIODevice::WriteOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to write to common configuration file!"));
    }
    else
    {
        QJsonDocument document;
        QJsonObject obj;
        QJsonValue checkForUpdates = QJsonValue(settings.checkForUpdates);
        QJsonValue latestServerAddress = QJsonValue(settings.latestServerAddress.toString());
        QJsonValue latestServerPort = QJsonValue(QString::number(settings.latestServerPort));
        QJsonValue debugMode = QJsonValue(settings.debugMode);

        obj["checkForUpdates"] = checkForUpdates;
        obj["latestServerAddress"] = latestServerAddress;
        obj["latestServerPort"] = latestServerPort;
        obj["debugMode"] = debugMode;
        document.setObject(obj);
        QString defaultSettingsString = QString(document.toJson());

        QDataStream out(&commonFile);
        out.setVersion(QDataStream::Qt_5_0);
        out << defaultSettingsString;
        commonFile.close();
    }

    QFile userInterfaceFile(QString(appDataFilePath + "/Settings/userInterface.flat"));
    if(!userInterfaceFile.exists())
    {
        log(LogType::Error, tr("User interface configuration file not found!"));
    }
    if(!userInterfaceFile.open(QIODevice::WriteOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to write to user interface configuration file!"));
    }
    else
    {
        QJsonDocument document;
        QJsonObject obj;
        QJsonValue language = QJsonValue(settings.language);
        QJsonValue username = QJsonValue(settings.username);
        QJsonValue windowFullSreen = QJsonValue(settings.windowFullScreen);
        QJsonValue windowMaximized = QJsonValue(settings.windowMaximized);

        obj["language"] = language;
        obj["username"] = username;
        obj["windowFullScreen"] = windowFullSreen;
        obj["windowMaximized"] = windowMaximized;
        document.setObject(obj);
        QString defaultSettingsString = QString(document.toJson());

        QDataStream out(&userInterfaceFile);
        out.setVersion(QDataStream::Qt_5_0);
        out << defaultSettingsString;
        userInterfaceFile.close();
    }

    QFile soundsFile(QString(appDataFilePath + "/Settings/sounds.flat"));
    if(!soundsFile.exists())
    {
        log(LogType::Error, tr("Sounds configuration file not found!"));
    }
    if(!soundsFile.open(QIODevice::WriteOnly))
    {
        err = true;
        log(LogType::Error, tr("Failed to write to sounds configuration file!"));
    }
    else
    {
        QJsonDocument document;
        QJsonObject obj;
        QJsonValue soundEnabled = QJsonValue(settings.soundEnabled);
        QJsonValue soundVolume = QJsonValue(QString::number(settings.soundVolume));

        obj["soundEnabled"] = soundEnabled;
        obj["soundVolume"] = soundVolume;
        document.setObject(obj);
        QString defaultSettingsString = QString(document.toJson());

        QDataStream out(&soundsFile);
        out.setVersion(QDataStream::Qt_5_0);
        out << defaultSettingsString;
        soundsFile.close();
    }

    return !err;
}

void MainWindow::showMultiplayerMenu()
{
    log(LogType::Info, tr("Showing multiplayer menu"));
    this->setWindowTitle(QString(tr("Multiplayer") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(multiplayerMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeight = 50;
    int buttonsSpacing = 5;

    QLabel *labelMultiplayerTitle = new QLabel(tr("Multiplayer"));
    labelMultiplayerTitle->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeight) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelMultiplayerTitle->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelMultiplayerTitle = gameScene->addWidget(labelMultiplayerTitle);
    proxyLabelMultiplayerTitle->setPos(0 - (proxyLabelMultiplayerTitle->widget()->width() / 2), 0 - (1 * buttonsHeight) - proxyLabelMultiplayerTitle->widget()->height());
    proxyItemList.append(proxyLabelMultiplayerTitle);

    QLabel *labelMultiplayerServers = new QLabel(tr("Saved servers"));
    labelMultiplayerServers->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelMultiplayerServers->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelMultiplayerServers = gameScene->addWidget(labelMultiplayerServers);
    proxyLabelMultiplayerServers->setPos(0 - buttonsWidth + (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelMultiplayerServers);

    listMultiplayerServers = new QListWidget;
    connect(listMultiplayerServers, &QListWidget::currentItemChanged, this, &MainWindow::onListMultiplayerServersItemChanged);
    for(int i = 0; i < savedServerList.length(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(savedServerList.at(i).icon);
        item->setText(savedServerList.at(i).name + " (" + savedServerList.at(i).address.toString() + ":" + QString::number(savedServerList.at(i).port) + ")");
        item->setData(1, QVariant(QString(savedServerList.at(i).address.toString() + ":" + QString::number(savedServerList.at(i).port))));
        listMultiplayerServers->addItem(item);
    }
    QGraphicsProxyWidget *proxyListMultiplayerServers = gameScene->addWidget(listMultiplayerServers);
    proxyListMultiplayerServers->setPos(0 - (buttonsWidth / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyListMultiplayerServers);

    QPushButton *buttonMultiplayerConnect = new GraphicButton(tr("Connect"), menuButtonBacgroundFile, (buttonsWidth / 2) - (buttonsSpacing / 2), buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiplayerConnect, &QPushButton::clicked, this, &MainWindow::onButtonMultiplayerConnectClicked);
    QGraphicsProxyWidget *proxyButtonMultiplayerConnect = gameScene->addWidget(buttonMultiplayerConnect);
    proxyButtonMultiplayerConnect->setPos(0 + (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiplayerConnect);

    QPushButton *buttonMultiplayerAdd = new GraphicButton(tr("Add"), menuButtonBacgroundFile, (buttonsWidth / 2) - (buttonsSpacing / 2), buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiplayerAdd, &QPushButton::clicked, this, &MainWindow::onButtonMultiplayerAddClicked);
    QGraphicsProxyWidget *proxyButtonMultiplayerAdd = gameScene->addWidget(buttonMultiplayerAdd);
    proxyButtonMultiplayerAdd->setPos(0 + buttonsSpacing + (buttonsWidth / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiplayerAdd);

    QLabel *labelMultiplayerLanServers = new QLabel(tr("LAN servers"));
    labelMultiplayerLanServers->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelMultiplayerLanServers->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelMultiplayerLanServers = gameScene->addWidget(labelMultiplayerLanServers);
    proxyLabelMultiplayerLanServers->setPos(0 - buttonsWidth + (buttonsSpacing / 2), 4 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelMultiplayerLanServers);

    listMultiplayerLANServers = new QListWidget;
    connect(listMultiplayerLANServers, &QListWidget::currentItemChanged, this, &MainWindow::onListMultiplayerLANServersItemChanged);
    QListWidgetItem *item = new QListWidgetItem;
    item->setIcon(QIcon("Sources/System/serverDefaultIcon.png"));
    item->setText("Please select ...");
    item->setToolTip(tr("If there are no other items, no servers in your LAN are running!"));
    item->setData(1, QVariant(QString(QHostAddress(QHostAddress::Broadcast).toString() + ":" + QString::number(-1))));
    listMultiplayerLANServers->addItem(item);
    QGraphicsProxyWidget *proxyListMultiplayerLANServers = gameScene->addWidget(listMultiplayerLANServers);
    proxyListMultiplayerLANServers->setPos(0 - (buttonsWidth / 2), 5 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyListMultiplayerLANServers);

    QPushButton *buttonMultiplayerConnectLAN = new GraphicButton(tr("Connect to LAN"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiplayerConnectLAN, &QPushButton::clicked, this, &MainWindow::onButtonMultiplayerConnectLANClicked);
    QGraphicsProxyWidget *proxyButtonMultiplayerConnectLAN = gameScene->addWidget(buttonMultiplayerConnectLAN);
    proxyButtonMultiplayerConnectLAN->setPos(0 + (buttonsSpacing / 2), 4 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiplayerConnectLAN);

    QPushButton *buttonMultiplayerCreateServer = new GraphicButton(tr("Create server"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiplayerCreateServer, &QPushButton::clicked, this, &MainWindow::onButtonMultiplayerCreateServerClicked);
    QGraphicsProxyWidget *proxyButtonMultiplayerCreateServer = gameScene->addWidget(buttonMultiplayerCreateServer);
    proxyButtonMultiplayerCreateServer->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiplayerCreateServer);

    QPushButton *buttonMultiplayerBack = new GraphicButton(tr("Back"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonMultiplayerBack, &QPushButton::clicked, this, &MainWindow::onButtonMultiplayerBackClicked);
    QGraphicsProxyWidget *proxyButtonMultiplayerBack = gameScene->addWidget(buttonMultiplayerBack);
    proxyButtonMultiplayerBack->setPos(0 + (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonMultiplayerBack);

    listenForBroadcast();
}

void MainWindow::showAddServerMenu()
{
    log(LogType::Info, tr("Showing add server menu"));
    this->setWindowTitle(QString(tr("Add server") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(addServerMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeight = 50;
    int buttonsSpacing = 5;

    QLabel *labelAddServerName = new QLabel(tr("Server name"));
    labelAddServerName->setStyleSheet(QString("font-weight: bold;"
                                                    "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                    "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                    "height: " + QString::number(buttonsHeight) + "px;"
                                                    "font-family:" + menuButtonFontFamily + ";"));
    labelAddServerName->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelAddServerName = gameScene->addWidget(labelAddServerName);
    proxyLabelAddServerName->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelAddServerName);

    lineAddServerName = new QLineEdit;
    lineAddServerName->setPlaceholderText(tr("Server name"));
    connect(lineAddServerName, &QLineEdit::returnPressed, this, &MainWindow::onLineAddServerNameReturnPressed);
    QGraphicsProxyWidget *proxyLineAddServerName = gameScene->addWidget(lineAddServerName);
    proxyLineAddServerName->setPos(0 + (buttonsSpacing / 2), 0 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLineAddServerName);

    QLabel *labelAddServerAddress = new QLabel(tr("Server address"));
    labelAddServerAddress->setStyleSheet(QString("font-weight: bold;"
                                                    "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                    "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                    "height: " + QString::number(buttonsHeight) + "px;"
                                                    "font-family:" + menuButtonFontFamily + ";"));
    labelAddServerAddress->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelAddServerAddress = gameScene->addWidget(labelAddServerAddress);
    proxyLabelAddServerAddress->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelAddServerAddress);

    lineAddServerAddress = new QLineEdit;
    lineAddServerAddress->setPlaceholderText(tr("Server address"));
    connect(lineAddServerAddress, &QLineEdit::returnPressed, this, &MainWindow::onLineAddServerAddressReturnPressed);
    QGraphicsProxyWidget *proxyLineAddServerAddress = gameScene->addWidget(lineAddServerAddress);
    proxyLineAddServerAddress->setPos(0 + (buttonsSpacing / 2), 1 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLineAddServerAddress);

    QLabel *labelAddServerPort = new QLabel(tr("Server port"));
    labelAddServerPort->setStyleSheet(QString("font-weight: bold;"
                                                    "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                    "font-size: " + QString::number(buttonsHeight / 2) + "px;"
                                                    "height: " + QString::number(buttonsHeight) + "px;"
                                                    "font-family:" + menuButtonFontFamily + ";"));
    labelAddServerPort->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelAddServerPort = gameScene->addWidget(labelAddServerPort);
    proxyLabelAddServerPort->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 2 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyLabelAddServerPort);

    spinAddServerPort = new QSpinBox;
    spinAddServerPort->setMinimum(1025);
    spinAddServerPort->setMaximum(65535);
    spinAddServerPort->setValue(5123);
    connect(spinAddServerPort, SIGNAL(valueChanged(int)), this, SLOT(onSpinAddServerPortValueChanged(int)));
    QGraphicsProxyWidget *proxySpinAddServerPort = gameScene->addWidget(spinAddServerPort);
    proxySpinAddServerPort->setPos(0 + (buttonsSpacing / 2), 2 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxySpinAddServerPort);

    QPushButton *buttonAddServerAdd = new GraphicButton(tr("Add server"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonAddServerAdd, &QPushButton::clicked, this, &MainWindow::onButtonAddServerAddClicked);
    QGraphicsProxyWidget *proxyButtonAddServerAdd = gameScene->addWidget(buttonAddServerAdd);
    proxyButtonAddServerAdd->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonAddServerAdd);

    QPushButton *buttonAddServerBack = new GraphicButton(tr("Back"), menuButtonBacgroundFile, buttonsWidth, buttonsHeight, menuTextColor, menuButtonFontFamily);
    connect(buttonAddServerBack, &QPushButton::clicked, this, &MainWindow::onButtonAddServerBackClicked);
    QGraphicsProxyWidget *proxyButtonAddServerBack = gameScene->addWidget(buttonAddServerBack);
    proxyButtonAddServerBack->setPos(0 + (buttonsSpacing / 2), 6 * (buttonsHeight + buttonsSpacing));
    proxyItemList.append(proxyButtonAddServerBack);
}

void MainWindow::addServerSave(QString name, QString address, int port)
{
    dnsLookupTmpServerInfo.name = name;
    dnsLookupTmpServerInfo.port = port;

    if(QHostAddress(address).isNull())
    {
        QLabel *labelAddServerWaitDns = new QLabel(tr("Waiting for dns lookup ..."));
        labelAddServerWaitDns->setStyleSheet(QString("font-weight: bold;"
                                                        "color: red;"
                                                        "font-size: 50px;"
                                                        "height: 100px;"
                                                        "font-family:" + menuButtonFontFamily + ";"));
        labelAddServerWaitDns->setAttribute(Qt::WA_TranslucentBackground);
        QGraphicsProxyWidget *proxyLabelAddServerWaitDns = gameScene->addWidget(labelAddServerWaitDns);
        proxyLabelAddServerWaitDns->setPos(0 - labelAddServerWaitDns->width(), 0);
        proxyItemList.append(proxyLabelAddServerWaitDns);

        QHostInfo::lookupHost(address, this, SLOT(onDnsLookup(QHostInfo)));
        QTimer::singleShot(15000, this, &MainWindow::onDnsLookupTimeout);//15 sekund
    }
    else
    {
        dnsLookupTmpServerInfo.address = QHostAddress(address);
        addServerSaveFinal();
    }
}

void MainWindow::addServerSaveFinal()
{
    savedServerList.append(dnsLookupTmpServerInfo);

    log(LogType::Info, tr("New server saved"));
    showMultiplayerMenu();
}

void MainWindow::onDnsLookupTimeout()
{
    qDebug() << "dns lookupt timeout";
}

void MainWindow::showCreateServerMenu()
{
    log(LogType::Info, tr("Showing create server menu"));
    this->setWindowTitle(QString(tr("Create server") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(createServerMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeigh = 50;
    int buttonsSpacing = 5;

    QLabel *labelCreateServerTitle = new QLabel(tr("Create server"));
    labelCreateServerTitle->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerTitle->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerTitle = gameScene->addWidget(labelCreateServerTitle);
    proxyLabelCreateServerTitle->setPos(0 - (proxyLabelCreateServerTitle->widget()->width() / 2), 0 - (1 * buttonsHeigh) - proxyLabelCreateServerTitle->widget()->height());
    proxyItemList.append(proxyLabelCreateServerTitle);

    QLabel *labelCreateServerName = new QLabel(tr("Server name"));
    labelCreateServerName->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerName->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerName = gameScene->addWidget(labelCreateServerName);
    proxyLabelCreateServerName->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 0 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLabelCreateServerName);

    lineCreateServerName = new QLineEdit;
    lineCreateServerName->setPlaceholderText(tr("Server name"));
    connect(lineCreateServerName, &QLineEdit::returnPressed, this, &MainWindow::onLineCreateServerNameReturnPressed);
    QGraphicsProxyWidget *proxyLineCreateServerName = gameScene->addWidget(lineCreateServerName);
    proxyLineCreateServerName->setPos(0 - (buttonsSpacing / 2), 0 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLineCreateServerName);

    QLabel *labelCreateServerAddress = new QLabel(tr("Server address"));
    labelCreateServerAddress->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerAddress->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerAddress = gameScene->addWidget(labelCreateServerAddress);
    proxyLabelCreateServerAddress->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 1 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLabelCreateServerAddress);

    comboCreateServerAddress = new QComboBox;
    comboCreateServerAddress->addItem(QIcon(), "All interfaces", QVariant(QHostAddress(QHostAddress::Any).toString()));
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        comboCreateServerAddress->addItem(QIcon(), address.toString(), QVariant(address.toString()));
    }
    connect(comboCreateServerAddress, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboCreateServerAddressChanged(int)));
    QGraphicsProxyWidget *proxyComboCreateServerAddress = gameScene->addWidget(comboCreateServerAddress);
    proxyComboCreateServerAddress->setPos(0 - (buttonsSpacing / 2), 1 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyComboCreateServerAddress);

    QLabel *labelCreateServerPort = new QLabel(tr("Server port"));
    labelCreateServerPort->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerPort->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerPort = gameScene->addWidget(labelCreateServerPort);
    proxyLabelCreateServerPort->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 2 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLabelCreateServerPort);

    spinCreateServerPort = new QSpinBox;
    spinCreateServerPort->setMinimum(1024);
    spinCreateServerPort->setMaximum(65535);
    spinCreateServerPort->setValue(5123);
    connect(spinCreateServerPort, SIGNAL(valueChanged(int)), this, SLOT(onSpinCreateServerPortValueChanged(int)));
    QGraphicsProxyWidget *proxySpinCreateServerPort = gameScene->addWidget(spinCreateServerPort);
    proxySpinCreateServerPort->setPos(0 + (buttonsSpacing / 2), 2 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxySpinCreateServerPort);

    QLabel *labelCreateServerNumberOfClients = new QLabel(tr("Number of clients"));
    labelCreateServerNumberOfClients->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerNumberOfClients->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerNumberOfClients = gameScene->addWidget(labelCreateServerNumberOfClients);
    proxyLabelCreateServerNumberOfClients->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 3 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLabelCreateServerNumberOfClients);

    spinCreateServerNumberOfClients = new QSpinBox;
    spinCreateServerNumberOfClients->setMinimum(1);
    spinCreateServerNumberOfClients->setMaximum(16);
    spinCreateServerNumberOfClients->setValue(5);
    connect(spinCreateServerNumberOfClients, SIGNAL(valueChanged(int)), this, SLOT(onSpinCreateServerNumberOfClientsValueChanged(int)));
    QGraphicsProxyWidget *proxySpinCreateServerNumberOfClients = gameScene->addWidget(spinCreateServerNumberOfClients);
    proxySpinCreateServerNumberOfClients->setPos(0 + (buttonsSpacing / 2), 3 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxySpinCreateServerNumberOfClients);

    QLabel *labelCreateServerPassword = new QLabel(tr("Password"));
    labelCreateServerPassword->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelCreateServerPassword->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelCreateServerPassword = gameScene->addWidget(labelCreateServerPassword);
    proxyLabelCreateServerPassword->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 4 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLabelCreateServerPassword);

    lineCreateServerPassword = new QLineEdit;
    lineCreateServerPassword->setPlaceholderText(tr("Server password"));
    connect(lineCreateServerPassword, &QLineEdit::returnPressed, this, &MainWindow::onLineCreateServerPasswordReturnPressed);
    QGraphicsProxyWidget *proxyLineCreateServerPassword = gameScene->addWidget(lineCreateServerPassword);
    proxyLineCreateServerPassword->setPos(0 - (buttonsSpacing / 2), 4 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyLineCreateServerPassword);

    QPushButton *buttonCreateServerStart = new GraphicButton(tr("Start"), menuButtonBacgroundFile, buttonsWidth, buttonsHeigh, menuTextColor, menuButtonFontFamily);
    connect(buttonCreateServerStart, &QPushButton::clicked, this, &MainWindow::onButtonCreateServerStartClicked);
    QGraphicsProxyWidget *proxyButtonCreateServerStart = gameScene->addWidget(buttonCreateServerStart);
    proxyButtonCreateServerStart->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 6 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyButtonCreateServerStart);

    QPushButton *buttonCreateServerBack = new GraphicButton(tr("Back"), menuButtonBacgroundFile, buttonsWidth, buttonsHeigh, menuTextColor, menuButtonFontFamily);
    connect(buttonCreateServerBack, &QPushButton::clicked, this, &MainWindow::onButtonCreateServerBackClicked);
    QGraphicsProxyWidget *proxyButtonCreateServerBack = gameScene->addWidget(buttonCreateServerBack);
    proxyButtonCreateServerBack->setPos(0 + (buttonsSpacing / 2), 6 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyButtonCreateServerBack);
}

void MainWindow::showLobbyMenu()
{
    log(LogType::Info, tr("Showing lobby menu"));
    this->setWindowTitle(QString(tr("Lobby") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(lobbyMenuBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeigh = 50;
    int buttonsSpacing = 5;

    QCheckBox *checkLobbyReady = new QCheckBox;
    checkLobbyReady->setChecked(false);
    checkLobbyReady->setText(tr("I am ready!"));
    checkLobbyReady->setStyleSheet(QString("font-weight: bold;"
                                           "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                           "font-size: " + QString::number(buttonsHeigh / 2) + "px;"
                                           "font-family:" + menuButtonFontFamily + ";"));
    checkLobbyReady->setAttribute(Qt::WA_TranslucentBackground);
    connect(checkLobbyReady, &QCheckBox::toggled, this, &MainWindow::onCheckLobbyReadyToggled);
    QGraphicsProxyWidget *proxyCheckLobbyReady = gameScene->addWidget(checkLobbyReady);
    proxyCheckLobbyReady->setPos(0 - buttonsWidth - (buttonsSpacing / 2), 6 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyCheckLobbyReady);

    QString buttonText;
    if(serverIsActive)
    {
        buttonText = tr("Stop server");
    }
    else
    {
        buttonText = tr("Disconnect");
    }
    QPushButton *buttonLobbyDisconnect = new GraphicButton(buttonText, menuButtonBacgroundFile, buttonsWidth, buttonsHeigh, menuTextColor, menuButtonFontFamily);
    connect(buttonLobbyDisconnect, &QPushButton::clicked, this, &MainWindow::onButtonLobbyDisconnectClicked);
    QGraphicsProxyWidget *proxyButtonLobbyDisconnect = gameScene->addWidget(buttonLobbyDisconnect);
    proxyButtonLobbyDisconnect->setPos(0 + (buttonsSpacing / 2), 6 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyButtonLobbyDisconnect);
}

void MainWindow::showDeveloperInfo()
{
    log(LogType::Info, tr("Showing developer info"));
    this->setWindowTitle(QString(tr("Developers") + " | The Flat"));

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(developerInfoBackgroundFile)));

    int buttonsWidth = 300;
    int buttonsHeigh = 50;
    int buttonsSpacing = 5;

    QLabel *labelDeveloperInfoTitle = new QLabel(tr("Developers"));
    labelDeveloperInfoTitle->setStyleSheet(QString("font-weight: bold;"
                                                 "color: rgb(" + QString::number(menuTextColor.red()) + ", " + QString::number(menuTextColor.green()) + ", " + QString::number(menuTextColor.blue()) + ");"
                                                 "font-size: " + QString::number(buttonsHeigh) + "px;"
                                                 "font-family:" + menuButtonFontFamily + ";"));
    labelDeveloperInfoTitle->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsProxyWidget *proxyLabelDeveloperInfoTitle = gameScene->addWidget(labelDeveloperInfoTitle);
    proxyLabelDeveloperInfoTitle->setPos(0 - (proxyLabelDeveloperInfoTitle->widget()->width() / 2), 0 - (1 * buttonsHeigh) - proxyLabelDeveloperInfoTitle->widget()->height());
    proxyItemList.append(proxyLabelDeveloperInfoTitle);

    QPushButton *buttonDeveloperInfoBack = new GraphicButton(tr("Back"), menuButtonBacgroundFile, buttonsWidth, buttonsHeigh, menuTextColor, menuButtonFontFamily);
    connect(buttonDeveloperInfoBack, &QPushButton::clicked, this, &MainWindow::onButtonDeveloperInfoBackClicked);
    QGraphicsProxyWidget *proxyButtonDeveloperInfoBack = gameScene->addWidget(buttonDeveloperInfoBack);
    proxyButtonDeveloperInfoBack->setPos(0 - (buttonsWidth / 2), 6 * (buttonsHeigh + buttonsSpacing));
    proxyItemList.append(proxyButtonDeveloperInfoBack);
}

void MainWindow::showGameView()
{
    log(LogType::Info, tr("Showing game view"));
    this->setWindowTitle("The Flat");

    removeProxyItems();
    gameScene->setBackgroundBrush(QBrush(QPixmap(gameViewBackgroundFile)));
}

void MainWindow::showGameTile(QString typeName, int x, int y, int width, QString file)
{
    GraphicTile *newTile = new GraphicTile(file, width);
    newTile->setPos(x * width, y * width);
    GameTile tempGameTile;
    tempGameTile.x = x;
    tempGameTile.y = y;
    tempGameTile.graphicItemPointer = newTile;
    tempGameTile.typeName = typeName;
    gameTileList.append(&tempGameTile);
    gameScene->addItem(newTile);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(gameIsActive)
    {
        QMessageBox::StandardButton question = QMessageBox::question(this, tr("Warning"), tr("Game is still running! Are you sure you want to quit?"), QMessageBox::Yes | QMessageBox::No);
        if(question == QMessageBox::Yes)
        {
            log(LogType::Info, tr("User accepted close event. Quiting application"));
            event->accept();
        }
        else
        {
            log(LogType::Info, tr("User rejected close event."));
            event->ignore();
        }
    }
    else
    {
        log(LogType::Info, tr("Quiting application"));
        event->accept();
    }
}

void MainWindow::log(LogType type, QString text)
{
    QString typeText = "_";
    if(type == LogType::Info)
    {
        typeText = "INFO";
    }
    else if(type == LogType::Warning)
    {
        typeText = "WARNING";
    }
    else if(type == LogType::Error)
    {
        typeText = "ERROR";
    }
    else if(type == LogType::Game)
    {
        typeText = "GAME";
    }
    else if(type == LogType::Server)
    {
        typeText = "SERVER";
    }
    else if(type == LogType::Player)
    {
        typeText = "PLAYER";
    }
    qDebug() << typeText << text;
    QTextStream out(logFile);
    QString timestamp = QDateTime::currentDateTime().toString("dd.MM.yyyy-HH:mm:ss.zzz");
    out << timestamp << " [" << typeText << "] " << text << endl;
}

void MainWindow::updateCheckFinished(QNetworkReply *reply)
{
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject obj = doc.object();
    if(obj.contains("latest_version") && obj["latest_version"].isString())
    {
        QString latest_version = obj["latest_version"].toString();
        if(latest_version == version)
        {
            log(LogType::Info, tr("Update check complete! Application is the latest version"));
        }
        else
        {
            log(LogType::Info, tr("Update check complete! Some updates was found"));
            QMessageBox::StandardButton question = QMessageBox::question(this, tr("Updates found!"), tr("Newer version of application was found, do you want to install uodates now?"), QMessageBox::Yes | QMessageBox::No);
            if(question == QMessageBox::Yes)
            {
                log(LogType::Info, tr("User accepted update installation"));
            }
            else
            {
                log(LogType::Info, tr("User declined update installation"));
            }
        }
    }
}

void MainWindow::downloadFinished(QNetworkReply *reply)
{
    QString fileUrl = reply->url().toString();
    QString filePath = fileUrl.remove(fileServerUrl);

    log(LogType::Info, QString(tr("Finished Downloading file: ") + filePath));

    QFile newFile;
    newFile.setFileName(QString("Sources/" + filePath));
    if(!newFile.open(QIODevice::WriteOnly))
    {
        log(LogType::Error, QString(tr("Failed to save downloaded file: ") + filePath));
    }
    else
    {
        newFile.write(reply->readAll());
        newFile.close();
    }

    downloadFileCount--;
    if(downloadFileCount == 0)
    {
        log(LogType::Info, QString(tr("Downloading of files finished")));
        QMessageBox::StandardButton info = QMessageBox::information(this, tr("Info"), tr("Downloading of files finished. Please restart application to load all resources correctly."), QMessageBox::Ok);
    }
}

void MainWindow::onDnsLookup(QHostInfo info)
{
    qDebug() << "dns lookup done";
    if(info.error() == QHostInfo::NoError)
    {
        foreach (const QHostAddress &address, info.addresses())
        {
            dnsLookupTmpServerInfo.address = address;
        }
        addServerSaveFinal();
    }
}

void MainWindow::onBackgroundAudioPlaylistPositionChanged(int position)
{
    qDebug() << position;
}

void MainWindow::onButtonSinglePlayerClicked()
{
    log(LogType::Info, tr("Button Singleplayer in main menu was clicked"));
}

void MainWindow::onButtonMultiPlayerClicked()
{
    log(LogType::Info, tr("Button Multiplayer in main menu was clicked"));
    showMultiplayerMenu();
}

void MainWindow::onButtonSettingsClicked()
{
    log(LogType::Info, tr("Button Settings in main menu was clicked"));
    showSettingsMenu();
}

void MainWindow::onButtonDeveloperInfoClicked()
{
    log(LogType::Info, tr("Button Developer info in main menu was clicked"));
    showDeveloperInfo();
}

void MainWindow::onButtonQuitClicked()
{
    log(LogType::Info, tr("Button Quit in main menu was clicked"));
    this->close();
}

void MainWindow::onButtonSettingsSaveClicked()
{
    log(LogType::Info, tr("Button Save in settings menu was clicked"));
    saveSettings();
}

void MainWindow::onButtonSettingsBackClicked()
{
    log(LogType::Info, tr("Button Back in settings menu was clicked"));
    if(someSettingsWasChanged == true)
    {
        QMessageBox::StandardButton question = QMessageBox::question(this, tr("Warning"), tr("You have unsaved changes. Do you want to save them?"), QMessageBox::Yes | QMessageBox::No);
        if(question == QMessageBox::Yes)
        {
            saveSettings();
        }
    }
    someSettingsWasChanged = false;
    showMainMenu();
}

void MainWindow::onButtonMultiplayerCreateServerClicked()
{
    log(LogType::Info, tr("Button Create server in multiplayer menu was clicked"));
    stopListeningForBroadcast();
    showCreateServerMenu();
}

void MainWindow::onButtonMultiplayerBackClicked()
{
    log(LogType::Info, tr("Button Back in multiplayer menu was clicked"));
    stopListeningForBroadcast();
    showMainMenu();
}

void MainWindow::onCheckSettingsSoundEnabledToggled(bool checked)
{
    QString state;
    if(checked)
    {
        state = "Enabled";
    }
    else
    {
        state = "Disabled";
    }
    log(LogType::Info, tr("Checkbox Sound enabled in settings menu has changed state to ") + state);
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onCheckSettingsWindowMaximizedToggled(bool checked)
{
    QString state;
    if(checked)
    {
        state = "Enabled";
    }
    else
    {
        state = "Disabled";
    }
    log(LogType::Info, tr("Checkbox Window maximized in settings menu has changed state to ") + state);
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onCheckSettingsCheckForUpdatesToggled(bool checked)
{
    QString state;
    if(checked)
    {
        state = "Enabled";
    }
    else
    {
        state = "Disabled";
    }
    log(LogType::Info, tr("Checkbox Check for updates in settings menu has changed state to ") + state);
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onCheckSettingsWindowFullScreenToggled(bool checked)
{
    QString state;
    if(checked)
    {
        state = "Enabled";
    }
    else
    {
        state = "Disabled";
    }
    log(LogType::Info, tr("Checkbox Window fullscreen in settings menu has changed state to ") + state);
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onSliderSettingsSoundVolumeValueChanged(int value)
{
    log(LogType::Info, tr("Slider Sound volume in settings menu has changed value to ") + QString::number(value));
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onLineSettingsUsernameReturnPressed()
{
    log(LogType::Info, tr("Return on line edit Username in settings menu was clicked"));
    someSettingsWasChanged = true;
    settingsMenuUpdateSaveButton();
}

void MainWindow::onListMultiplayerServersItemChanged(QListWidgetItem *item)
{
    log(LogType::Info, tr("Item in list Servers in multiplayer menu has changed item to ") + item->text());
    qDebug() << "item data" << item->data(1).toString();
}

void MainWindow::onListMultiplayerLANServersItemChanged(QListWidgetItem *item)
{
    log(LogType::Info, tr("Item in list LAN Servers in multiplayer menu has changed item to ") + item->text());
    qDebug() << "item data" << item->data(1).toString();
}

void MainWindow::onButtonMultiplayerConnectClicked()
{
    log(LogType::Info, tr("Button Connect in multiplayer menu was clicked"));
    QString itemData = listMultiplayerServers->selectedItems().at(0)->data(1).toString();
    QStringList itemDataList = itemData.split(":");
    QHostAddress address = QHostAddress(itemDataList.at(0));
    int port = itemDataList.at(1).toInt();
    connectToServer(address, port);
}

void MainWindow::onButtonMultiplayerAddClicked()
{
    log(LogType::Info, tr("Button Add in multiplayer menu was clicked"));
    stopListeningForBroadcast();
    showAddServerMenu();
}

void MainWindow::onButtonMultiplayerConnectLANClicked()
{
    log(LogType::Info, tr("Button Connect LAN in multiplayer menu was clicked"));
}

void MainWindow::onButtonAddServerAddClicked()
{
    log(LogType::Info, tr("Button Add in add server menu was clicked"));
    addServerSave(lineAddServerName->text(), lineAddServerAddress->text(), spinAddServerPort->value());
}

void MainWindow::onButtonAddServerBackClicked()
{
    log(LogType::Info, tr("Button Back in add server menu was clicked"));
    QMessageBox::StandardButton question = QMessageBox::question(this, tr("Discard changes?"), tr("Are you sure you want to discard changes and return to multiplayer menu?"), QMessageBox::Yes | QMessageBox::No);
    if(question == QMessageBox::Yes)
    {
        log(LogType::Info, tr("User accepted close action"));
        showMultiplayerMenu();
    }
    else
    {
        log(LogType::Info, tr("User declined close action"));
    }
}

void MainWindow::onLineAddServerNameReturnPressed()
{
    log(LogType::Info, tr("Return on line edit name in add server menu was pressed"));
}

void MainWindow::onLineAddServerAddressReturnPressed()
{
    log(LogType::Info, tr("Return on line edit address in add server menu was pressed"));
}

void MainWindow::onSpinAddServerPortValueChanged(int value)
{
    log(LogType::Info, tr("Spin Port in add server menu has changed value to ") + QString::number(value));
}

void MainWindow::onButtonCreateServerStartClicked()
{
    log(LogType::Info, tr("Button Start in create server menu was clicked"));

    runningServerInfo.name = lineCreateServerName->text();
    runningServerInfo.address = QHostAddress(comboCreateServerAddress->currentData().toString());
    runningServerInfo.port = spinCreateServerPort->value();
    runningServerInfo.numberOfClients = spinCreateServerNumberOfClients->value();

    runningServerPassword = lineCreateServerPassword->text();

    log(LogType::Info, tr("Starting server"));

    if(startServer())
    {
        serverIsActive = true;
        showLobbyMenu();
        startBroadcastInfo();
    }
    else
    {
        runningServerInfo.name = "undefinied";
        runningServerInfo.quickDescribtion = "";
        runningServerInfo.address = QHostAddress::Broadcast;
        runningServerInfo.port = -1;
        runningServerInfo.numberOfClients = -1;

        runningServerPassword = "1234";
    }
}

void MainWindow::onButtonCreateServerBackClicked()
{
    log(LogType::Info, tr("Button Back in create server menu was clicked"));
    showMultiplayerMenu();
}

void MainWindow::onComboCreateServerAddressChanged(int index)
{
    log(LogType::Info, tr("Combo Server address in create server menu has changed index to ") + QString::number(index));
}

void MainWindow::onLineCreateServerNameReturnPressed()
{
    log(LogType::Info, tr("Return on line edit server name in create server menu was pressed"));
}

void MainWindow::onSpinCreateServerPortValueChanged(int value)
{
    log(LogType::Info, tr("Spin Server port in create server menu has changed value to ") + QString::number(value));
}

void MainWindow::onSpinCreateServerNumberOfClientsValueChanged(int value)
{
    log(LogType::Info, tr("Spin Number of clients in create server menu has changed value to ") + QString::number(value));
}

void MainWindow::onLineCreateServerPasswordReturnPressed()
{
    log(LogType::Info, tr("Return on line edit server password in create server menu was pressed"));
}

void MainWindow::onCheckLobbyReadyToggled(bool checked)
{
    QString state;
    if(checked)
    {
        state = "Enabled";
    }
    else
    {
        state = "Disabled";
    }
    log(LogType::Info, tr("Checkbox Ready in lobby has changed state to ") + state);

    if(serverIsActive)
    {
        if(serverCheckClientsReady())
        {
            serverStartingGameTimeout();
            QTimer::singleShot(1000, this, &MainWindow::serverStartingGameTimeout);
            QTimer::singleShot(2000, this, &MainWindow::serverStartingGameTimeout);
            QTimer::singleShot(3000, this, &MainWindow::serverStartingGameTimeout);
            QTimer::singleShot(4000, this, &MainWindow::serverStartingGameTimeout);
            QTimer::singleShot(5000, this, &MainWindow::serverStartGame);

            QByteArray data;
            QJsonDocument doc;
            QJsonObject obj;

            QJsonValue application = QJsonValue(QString("The Flat"));
            obj["application"] = application;
            QJsonValue action = QJsonValue(QString("starting game"));
            obj["action"] = action;
            QJsonArray clientList;
            for(int i = 0; i < serverClientList.length(); ++i)
            {
                QJsonObject clientObj;
                QJsonValue username = QJsonValue(serverClientList.at(i).username);

                clientObj["username"] = username;
                clientList.append(QJsonValue(clientObj));
            }
            obj["clientList"] = QJsonValue(clientList);
            doc.setObject(obj);
            data = doc.toJson();

            serverSendToAllClients(data);
            stopBroadcastInfo();
        }
    }
}

void MainWindow::onButtonLobbyDisconnectClicked()
{
    log(LogType::Info, tr("Button Disconnect in lobby was clicked"));
    QString dialogText;
    if(serverIsActive)
    {
        dialogText = tr("Stop server?");
    }
    else
    {
        dialogText = tr("Disconnect?");
    }
    QMessageBox::StandardButton question = QMessageBox::question(this, dialogText, tr("Are you sure you want to disconnect from this game?"), QMessageBox::Yes | QMessageBox::No);
    if(question == QMessageBox::Yes)
    {
        log(LogType::Info, tr("User accepted disconnect action"));
        if(serverIsActive)
        {
            stopServer();
            showMultiplayerMenu();
        }
        else
        {
            disconnectFromServer();
        }
    }
    else
    {
        log(LogType::Info, tr("User declined disconnect action"));
    }
}

void MainWindow::onButtonDeveloperInfoBackClicked()
{
    log(LogType::Info, tr("Button Back in developer info page was clicked"));
    showMainMenu();
}

void MainWindow::checkForUpdates()
{
    log(LogType::Info, tr("Checking updates"));
    updateCheckIsRunning = true;
    updateCheckRequest.setUrl(QUrl(updateCheckServerUrl));
    updateCheckManager.get(updateCheckRequest);
}

void MainWindow::showDebugConsoleDock()
{
    debugDock = new QDockWidget(tr("Debug console"), this);
    debugDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, debugDock);
}
