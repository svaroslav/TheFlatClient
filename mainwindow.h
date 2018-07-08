#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QStandardPaths>

#include <QMessageBox>
#include <QApplication>

#include <QTimer>
#include <QDateTime>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUdpSocket>

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>

#include <QSystemTrayIcon>
#include <QMenu>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

#include <QGraphicsScene>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QListWidget>
#include <QGraphicsProxyWidget>

#include <QDockWidget>

#include <QCloseEvent>

#include <QMediaPlayer>
#include <QMediaPlayerControl>
#include <QMediaPlaylist>

#include "gameview.h"
#include "player.h"
#include "graphictile.h"
#include "graphicbutton.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct Settings
    {
        bool checkForUpdates = true;

        bool windowMaximized = false;
        bool windowFullScreen = false;

        bool soundEnabled = true;
        int soundVolume = 50;

        QString language = "en_UK";
        QString username = "Player one";

        QHostAddress latestServerAddress = QHostAddress(QHostAddress::Broadcast);
        int latestServerPort = 5123;

        bool debugMode = false;
    };

    struct GameTile
    {
        int x = 0;
        int y = 0;
        GraphicTile *graphicItemPointer = nullptr;
        QString typeName = "undefinied";
    };

    struct ServerInfo
    {
        QString name = "undefinied";
        QString quickDescribtion = "";
        QIcon icon = QIcon("Sources/System/defaultServerIcon.png");
        QHostAddress address = QHostAddress::Broadcast;
        int port = -1;
        int numberOfClients = -1;
    };

    struct Client
    {
        QTcpSocket *pointer = nullptr;
        QString username = "undefinied";
        bool ready = false;
        int state = ClientState::Connecting;
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    enum LogType {Info, Warning, Error, Game, Server, Player};
    enum ClientState {Disconnected, Connecting, LoggingIn, Authenticated, InLobby, InGame, Spectator};

public slots:
    void log(LogType type, QString text);
    QString translate(QString text);

private slots:
    void updateCheckFinished(QNetworkReply*reply);

    void downloadFinished(QNetworkReply*reply);

    void onDnsLookup(QHostInfo info);

    void onBackgroundAudioPlaylistPositionChanged(int position);

    void onButtonSinglePlayerClicked();
    void onButtonMultiPlayerClicked();
    void onButtonSettingsClicked();
    void onButtonGameInfoClicked();
    void onButtonQuitClicked();

    void onButtonMultiplayerCreateServerClicked();
    void onButtonMultiplayerBackClicked();
    void onListMultiplayerServersItemChanged(QListWidgetItem *item);
    void onListMultiplayerLANServersItemChanged(QListWidgetItem *item);
    void onButtonMultiplayerConnectClicked();
    void onButtonMultiplayerAddClicked();
    void onButtonMultiplayerConnectLANClicked();

    void onButtonConnectingToServerCancelClicked();

    void onButtonAddServerAddClicked();
    void onButtonAddServerBackClicked();
    void onLineAddServerNameReturnPressed();
    void onLineAddServerAddressReturnPressed();
    void onSpinAddServerPortValueChanged(int value);

    void onCheckSettingsSoundEnabledToggled(bool checked);
    void onCheckSettingsWindowMaximizedToggled(bool checked);
    void onCheckSettingsCheckForUpdatesToggled(bool checked);
    void onCheckSettingsWindowFullScreenToggled(bool checked);
    void onSliderSettingsSoundVolumeValueChanged(int value);
    void onLineSettingsUsernameReturnPressed();
    void onButtonSettingsSaveClicked();
    void onButtonSettingsBackClicked();

    void onButtonCreateServerStartClicked();
    void onButtonCreateServerBackClicked();
    void onComboCreateServerAddressChanged(int index);
    void onLineCreateServerNameReturnPressed();
    void onSpinCreateServerPortValueChanged(int value);
    void onSpinCreateServerNumberOfClientsValueChanged(int value);
    void onLineCreateServerPasswordReturnPressed();

    void onCheckLobbyReadyToggled(bool checked);
    void onButtonLobbyDisconnectClicked();

    void onButtonGameInfoBackClicked();

    void trayMenuStopServer();

private:
    Ui::MainWindow *ui;

    QString version = "0.0.1";
    void checkForUpdates();
    QString updateCheckServerUrl = "http://192.168.0.8/theflat/index.php";
    QNetworkAccessManager updateCheckManager;
    QNetworkRequest updateCheckRequest;
    bool updateCheckIsRunning = false;

    void showDebugConsoleDock();
    QDockWidget *debugDock;

    QString appDataFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile *logFile;
    bool initLogFile();
    bool checkAppFiles();
    QStringList systemFileList = (QStringList()
                                    << "loading.gif"
                                    << "grass_block_icon.png"
                                    << "defaultServerIcon.png");
    QStringList soundsFileList = (QStringList()
                                    << "fade.mp3");
    QStringList texturesFileList = (QStringList()
                                    << "plank_oak.png"
                                    << "plank_dark_oak.png"
                                    << "plank_birch.png"
                                    << "plank_pine.png"
                                    << "plank_jungle.png"
                                    << "plank_accacia.png"
                                    << "grass.png"
                                    << "farmland.png"
                                    << "farmland_irigated.png"
                                    << "farmland_potatoe.png"
                                    << "farmland_carrot.png"
                                    << "farmland_wheat.png"
                                    << "farmland_melon_growth_one.png");
    Settings settings;

    QJsonArray currentLanguageTextObjectArray = QJsonArray();
    void loadLanguageFromFile(QString language);

    void downloadAppFile(QString fileName);
    QString fileServerUrl = "http://192.168.0.8/theflat/Sources/";
    QNetworkAccessManager downloadManager;
    QNetworkRequest downloadRequest;
    int downloadFileCount = 0;
    bool fileDownloadIsRunning = false;

    bool loadSettingsFromFile();
    int savesCount = 0;

    void setupSound();
    QMediaPlayer *backgroundAudioPlayer;
    QMediaPlaylist *backgroundAudioPlaylist;
    bool backgroundAudioLoadPlaylist();

    QTcpServer *server;
    void serverNewConnection();
    QList<Client> waitingClientList;
    QList<Client> serverClientList;
    void serverReadyRead();
    void processReadData(QByteArray data, QTcpSocket *clientPointer);
    void serverDisconnected();
    bool serverIsActive = false;
    bool startServer();
    void disconnectAllUsers();
    void stopServer();
    void promoteServer();
    QString promoteServerUrl = "http://192.168.0.8/theflat/servers/";
    QNetworkAccessManager promoteManager;
    QNetworkRequest promoteRequest;
    bool promoteIsWaiting = false;
    QTimer *broadcastTimer;
    QUdpSocket *broadcastSocket;
    ServerInfo runningServerInfo;
    QString runningServerPassword = "1234";
    void sendBroadcastInfo();
    void startBroadcastInfo();
    void stopBroadcastInfo();
    bool serverCheckClientsReady();
    void serverStartingGameTimeout();
    int serverStartingTimeoutInt = 0;
    void serverStartGame();
    void serverSendToAllClients(QByteArray data);
    void showServerTrayIcon();
    void serverTrayIconShowMessage(QString text, int durationMs, QIcon icon);
    void hideServerTrayIcon();
    QSystemTrayIcon *serverTrayIcon;
    QMenu *serverTrayIconMenu;

    ServerInfo getServerInfo();
    ServerInfo connectedServerInfo;

    QTcpSocket *clientSocket;
    void connectToServer(QHostAddress address, int port);
    bool connectingToServer = false;
    bool connectedToServer = false;
    void cancelConnectingToServer();
    void socketConnectedToServer();
    void connectingToServerTimeout();
    void listenForBroadcast();
    void stopListeningForBroadcast();
    void disconnectFromServer();
    void clientReadyRead();
    void clientProcessReadData(QByteArray data);
    QUdpSocket *broadcastClientSocket;
    void udpSocketPendingDatagram();
    QList<ServerInfo> savedServerList;
    QList<ServerInfo> LANServerList;
    ClientState clientConnectionState = ClientState::Disconnected;
    void sendDataToServer(QByteArray data);

    GameView *gameView;
    QGraphicsScene *gameScene;
    void setupGameView();

    QList<QGraphicsProxyWidget *> proxyItemList;
    void removeProxyItems();
    QColor menuTextColor = QColor(0, 0, 64);
    QColor disabledMenuTextColor = QColor(128, 128, 128);
    QString menuButtonBacgroundFile = "Sources/Textures/grass.png";
    QString menuButtonFontFamily = "Verdana";

    void showMainMenu();
    QString mainMenuBackgroundFile = "Sources/Textures/plank_oak.png";

    void showSettingsMenu();
    QString settingsMenuBackgroundFile = "Sources/Textures/plank_oak.png";
    QCheckBox *checkSettingsSoundEnabled;
    QCheckBox *checkSettingsWindowMaximized;
    QCheckBox *checkSettingsCheckForUpdates;
    QCheckBox *checkSettingsWindowFullScreen;
    QSlider *sliderSettingsSoundVolume;
    QLineEdit *lineSettingsUsername;
    void settingsMenuUpdateSaveButton();
    QGraphicsProxyWidget *proxyButtonSettingsSave;
    void saveSettings();
    bool saveSettingsToFile();
    bool someSettingsWasChanged = false;

    void showMultiplayerMenu();
    QString multiplayerMenuBackgroundFile = "Sources/Textures/plank_oak.png";
    QListWidget *listMultiplayerServers;
    QListWidget *listMultiplayerLANServers;

    void showConnectingToServerSign();

    void showAddServerMenu();
    QString addServerMenuBackgroundFile = "Sources/Textures/plank_oak.png";
    QLineEdit *lineAddServerName;
    QLineEdit *lineAddServerAddress;
    QSpinBox *spinAddServerPort;
    void addServerSave(QString name, QString address, int port);
    void addServerSaveFinal();
    void onDnsLookupTimeout();
    ServerInfo dnsLookupTmpServerInfo;

    void showCreateServerMenu();
    QString createServerMenuBackgroundFile = "Sources/Textures/plank_oak.png";
    QLineEdit *lineCreateServerName;
    QComboBox *comboCreateServerAddress;
    QSpinBox *spinCreateServerPort;
    QSpinBox *spinCreateServerNumberOfClients;
    QLineEdit *lineCreateServerPassword;

    void showLobbyMenu();
    QString lobbyMenuBackgroundFile = "Sources/Textures/plank_birch.png";

    void showGameInfo();
    QString gameInfoBackgroundFile = "Sources/Textures/plank_accacia.png";

    void showGameView();
    QString gameViewBackgroundFile = "Sources/Textures/plank_pine.png";

    void showGameTile(QString typeName, int x, int y, int width, QString file);

    QList<GameTile *> gameTileList;

    void closeEvent(QCloseEvent *event);
    bool gameIsActive = false;
};

#endif // MAINWINDOW_H
