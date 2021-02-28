using UnityEngine;
using System.Net.Sockets;
using System.IO;
using System;
using System.Threading;
using System.Collections.Generic;

/// <summary>
/// Class Client represents simple TCP Client
/// </summary>
public class Client : MonoBehaviour
{
    // Name of the client
    public string clientName;

    // Indicates if the client is the host of the game or not
    public bool isHost;

    // Inficates if the client is disconnected from the server
    public bool disconnected;

    // Indicates if the client is trying to connect to the server
    public bool connecting = true;

    // Indicates if the client received ping message from the server
    public bool receivedPingReply = false;

    // Indicates if the socket is ready to be used
    public bool socketReady;

    // Socket that the client uses for connection
    private TcpClient socket;

    // Stream that client uses for receiving messages
    private NetworkStream stream;

    // Writer that client uses to send messages
    private StreamWriter writer;

    // Reader that client uses to read messages from stream
    private StreamReader reader;

    // Indicates if the main thread should pause (message handling) while the client is trying to connect to server
    private bool mainThreadPaused;

    // Current state of the client
    private State state;

    // Ip address of the server (host)
    string host;

    // Port of the server
    int port;


    /// <summary>
    /// All the states that the client can be in
    /// </summary>
    private enum State
    {
        LOBBY, // client is in the lobby
        REQ, // client sent or recieved game request
        PLAYING // client is playing the game
    }

    // Is beeing called upon creation of client 
    // Sets the client no to be destroyed between scenes
    private void Start()
    {
        DontDestroyOnLoad(gameObject);
    }

    /// <summary>
    /// Connects to the server
    /// </summary>
    /// <param name="host">IPv4 address of the server</param>
    /// <param name="port">port of the server</param>
    public void ConnectToServer(string host, int port)
    {

        disconnected = false;
        mainThreadPaused = false;
        this.host = host;
        this.port = port;

        Thread t = new Thread(pingHandler);
        t.Start();



    }

    // Update is beeing called every frame
    /// <summary>
    /// Check if there is a data avaibale in the stream and reads them
    /// call OnIncomingData
    /// </summary>
    private void Update()
    {
        if (socketReady)
        {
            if (stream.DataAvailable)
            {
                string data = reader.ReadLine();
                if (data != null)
                    OnIncomingData(data);
            }
        }
    }

    /// <summary>
    /// Sends message to the server
    /// </summary>
    /// <param name="data">Message that should be sent to server</param>
    public void Send(string data)
    {
        if (!socketReady) return;

        writer.WriteLine(data);
        writer.Flush();
    }

    /// <summary>
    /// Reads incoming messages from the server and reacts accordingly
    /// </summary>
    /// <param name="data">Message that was received from server</param>
    private void OnIncomingData(string data)
    {
        // Splits message into tokens
        string[] tokens = data.Split('~');

        switch (tokens[0])
        {

            case "SPONG":
                receivedPingReply = true;
                break;

            case "SMOV":
                OthelloBoard.Instance.MakeMove(int.Parse(tokens[1]), int.Parse(tokens[2]), OthelloBoard.Instance.isWhiteTurn);
                break;

            case "SADDP":

                GameObject playerListObject = GameObject.Find("PlayerList");

                if (playerListObject != null)
                {
                    PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
                    playerList.AddPlayer(tokens[1], PlayerListItem.PlayerState.kReady);
                }

                GameManager.Instance.players.Add(tokens[1], PlayerListItem.PlayerState.kReady);
                break;

            case "SDELP":

                playerListObject = GameObject.Find("PlayerList");


                if (playerListObject != null)
                {
                    PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
                    playerList.DeletePlayer(tokens[1]);
                }

                GameManager.Instance.players.Remove(tokens[1]);
                break;

            case "SUPS":
                if (tokens[2] == "READY")
                {
                    GameManager.Instance.players[tokens[1]] = PlayerListItem.PlayerState.kReady;
                    playerListObject = GameObject.Find("PlayerList");

                    if (playerListObject != null)
                    {
                        PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
                        playerList.UpdatePlayer(tokens[1], PlayerListItem.PlayerState.kReady);
                    }
                }
                else
                {
                    GameManager.Instance.players[tokens[1]] = PlayerListItem.PlayerState.kBusy;

                    playerListObject = GameObject.Find("PlayerList");

                    if (playerListObject != null)
                    {
                        PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
                        playerList.UpdatePlayer(tokens[1], PlayerListItem.PlayerState.kBusy);
                    }
                }
                break;


            case "SREQ":
                Lobby lobby = GameObject.Find("Lobby").GetComponent<Lobby>();
                lobby.OpenPopupWindow(tokens[1]);
                break;

            case "SREQC":
                lobby = GameObject.Find("Lobby").GetComponent<Lobby>();
                lobby.closeWaitForOtherPlayer();
                lobby.closeRequestWindow();
                break;

            case "SRECOV":
                GameManager.Instance.RecoverGame(tokens[1], tokens[2] == "1", tokens[3] == "1");
                break;


            case "SSTARTG":
                GameManager.Instance.StartGame();
                break;

            case "SGAMEC":
                GameManager.Instance.EndGame();
                break;

            case "SGAMEI":
                OthelloBoard board = GameObject.Find("Board").GetComponent<OthelloBoard>();
                board.Alert(tokens[1]);
                break;

            default:
                break;
        }
    }

    // It's beeing called on application quit
    /// <summary>
    /// Closes connection to the server
    /// </summary>
    private void OnApplicationQuit()
    {
        CloseConnection();
    }

    // It's beeing called upon destruction of client 
    /// <summary>
    /// Closes connection to the server
    /// </summary>
    private void OnDestroy()
    {
        CloseConnection();
    }

    /// <summary>
    /// Closes connection to the server and
    /// sets the disconneted variable to true.
    /// Also closes reader, writer and socket.
    /// </summary>
    public void CloseConnection()
    {
        disconnected = true;
        if (!socketReady)
            return;

        socketReady = false;
        writer.Close();
        reader.Close();
        socket.Close();
    }

    /// <summary>
    /// Sends the Message and Name to the server
    /// Changes state to lobby
    /// </summary>
    public void sendName()
    {
        Send("CNAME~" + clientName);
        GameManager.Instance.AddJob(() =>
        {
            GameManager.Instance.DisplayLoby();
        });
        mainThreadPaused = false;
        state = State.LOBBY;

    }

    /// <summary>
    /// Thread that is handling sending and receiving ping messages to the server
    /// It also trys to reconnect after the client is disconnected (looses connection)
    /// </summary>
    private void pingHandler()
    {
        while (disconnected == false)
        {
            if (connecting)
            {
                try
                {
                    socket = new TcpClient(host, port);
                    stream = socket.GetStream();
                    writer = new StreamWriter(stream);
                    reader = new StreamReader(stream);
                    connecting = false;
                    socketReady = true;
                    sendName();
                }
                catch (Exception e)
                {
                    GameManager.Instance.AddJob(() =>
                    {
                        GameManager.Instance.displayConnectingScreenAlert("Error during connecting!", e.Message);
                    });
               
                }
            }
            else
            {
                try
                {
                    receivedPingReply = false;
                    Send("CPING");

                    for (int i = 0; i < 10; i++)
                    {
                        if (receivedPingReply == true)
                            break;
                        Thread.Sleep(1000);

                    }

                    // server is not responding

                    if (!receivedPingReply)
                    {

                        GameManager.Instance.AddJob(() =>
                        {
                            GameManager.Instance.GetBackToConnectionScreen();
                        });
                        connecting = true;
                    }
                }
                catch (Exception)
                {
                    GameManager.Instance.AddJob(() =>
                    {
                        GameManager.Instance.GetBackToConnectionScreen();
                    });
                    connecting = true;
                }
            }
            try
            {
                Thread.Sleep(2500);
            }
            catch (Exception e)
            {
                Debug.Log(e);
            }
        }

    }

}
