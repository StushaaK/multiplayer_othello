using UnityEngine;
using UnityEngine.UI;
using System;
using System.Linq;
using System.Collections.Generic;

/// <summary>
/// Class MenuController is keeping references to the parts of the menu and switches between them accordingly
/// It stores all the functions for menu buttons 
/// </summary>
public class MenuController : MonoBehaviour
{
    // Reference to the game object with main menu
    public GameObject mainMenu;

    // Reference to the game object with connection submenu
    public GameObject connectMenu;

    // Reference to the game object with lobby submenu
    public GameObject lobbyMenu;

    // Reference to the game object with alert
    public GameObject alert;

    // Reference to the game object with waiting panel
    public GameObject waitingPanel;

    // Reference to the game object with client prefab
    public GameObject clientPrefab;



    /// <summary>
    /// Changes screen to connection submenu and instaciates game manager by calling it
    /// </summary>
    public void ConnectButton()
    {
        _ = GameManager.Instance;
        mainMenu.SetActive(false);
        connectMenu.SetActive(true);
    }

    /// <summary>
    /// Changes screen to connection submenu and creates allert after client looses connection with th server
    /// </summary>
    public void getBackToConnectionScreenWithError()
    {
        mainMenu.SetActive(false);
        connectMenu.SetActive(true);

        alert.SetActive(true);
        GameObject title = alert.transform.GetChild(0).gameObject;
        GameObject text = alert.transform.GetChild(1).gameObject;

        title.GetComponent<Text>().text = "Disconnected from the server!";
        text.GetComponent<Text>().text = "Server stopped responding.\n" +
            "If you lost your connection during game, you have 60 seconds to reconnect.\n";

        waitingPanel.SetActive(true);
    }


    /// <summary>
    /// Displays alert with the title and content given as parameters
    /// </summary>
    /// <param name="titleText">Title of the alert window</param>
    /// <param name="content">Main text of the alert</param>
    public void displayAlert(string titleText, string content)
    {
        alert.SetActive(true);
        GameObject title = alert.transform.GetChild(0).gameObject;
        GameObject text = alert.transform.GetChild(1).gameObject;

        title.GetComponent<Text>().text = titleText;
        text.GetComponent<Text>().text = content;
    }

    /// <summary>
    /// Gets inputs from the connection submenu, validates them and using client class tries to connect to the server
    /// </summary>
    public void ConnectToServerButton()
    {

        string hostAdress = GameObject.Find("HostInput").GetComponent<InputField>().text;
        if (hostAdress == "")
            hostAdress = "127.0.0.1";
        else if(!ValidateIPv4(hostAdress))
        {
            displayAlert("Connection error!", "Entered ip address is invalid, please enter valid ip-address and try again!");
            return;
        }

        int hostPort;
        if (!int.TryParse(GameObject.Find("PortInput").GetComponent<InputField>().text, out hostPort))
            hostPort = 6321;
        else if(!ValidatePortNumer(hostPort))
        {
            displayAlert("Connection error!", "Entered port is invalid, please enter valid port between 0 and 65535 and try again!");
            return;
        }

        string username = GameObject.Find("UsernameInput").GetComponent<InputField>().text;

        if(!ValidateUsername(username))
        {
            displayAlert("Connection error!", "Entered username is invalid, please enter valid username!\nIt can't be empty and it must be one world.");
            return;
        }

        Client c = Instantiate(clientPrefab).GetComponent<Client>();
        c.clientName = username;
        c.ConnectToServer(hostAdress, hostPort);

        waitingPanel.SetActive(true);
    }

    /// <summary>
    /// Validates IPv4 address
    /// </summary>
    /// <param name="ipString">String with IPv4 address</param>
    /// <returns>true if the given address is valid, otherwise false</returns>
    public bool ValidateIPv4(string ipString)
    {
        if (String.IsNullOrWhiteSpace(ipString))
        {
            return false;
        }

        string[] splitValues = ipString.Split('.');
        if (splitValues.Length != 4)
        {
            return false;
        }

        byte tempParsingByte;

        return splitValues.All(r => byte.TryParse(r, out tempParsingByte));
    }

    /// <summary>
    /// Validates port give as a parameter
    /// </summary>
    /// <param name="port">Numberl, representing port of the server to which the client is trying to connect to</param>
    /// <returns>True if the port is in valid range, otherwise false</returns>
    public bool ValidatePortNumer(int port)
    {
        if (port < 0 || port > 65535)
            return false;

        return true;
    }

    /// <summary>
    /// Validates username given as a parameter
    /// </summary>
    /// <param name="username">Username that the client is trying to set for himself</param>
    /// <returns>True, if the username is not empty and is one word, otherwise false</returns>
    public bool ValidateUsername(string username)
    {
        if (username == null || username == "")
            return false;
        else if (username.Split(' ').Length != 1)
            return false;

        return true;
    }

    /// <summary>
    /// Displays error message with specific socket error that is beeing given as parameter
    /// </summary>
    /// <param name="alertTitle">Title of the alert window</param>
    /// <param name="alertText">Main text of the error</param>
    public void displaySocketError(string alertTitle, string alertText)
    {
        /*Client c = FindObjectOfType<Client>();
        if (c != null)
            Destroy(c.gameObject);*/

        alert.SetActive(true);
        GameObject title = alert.transform.GetChild(0).gameObject;
        GameObject text = alert.transform.GetChild(1).gameObject;

        title.GetComponent<Text>().text = alertTitle;
        text.GetComponent<Text>().text = alertText;

    }


    /// <summary>
    /// Function that handles back button in the lobby
    /// It's sends to message about user exiting the lobby to the server and destroys client
    /// </summary>
    public void LobbyBackButton()
    {
        GameManager.Instance.players.Clear();

        PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
        playerList.ClearPlayerList();

        Client c = FindObjectOfType<Client>();
        c.Send("CEXIT");

        if (c != null)
            Destroy(c.gameObject);

        mainMenu.SetActive(true);
        connectMenu.SetActive(false);
        lobbyMenu.SetActive(false);

    }

    /// <summary>
    /// Function that handles basic back button used in Connection sub menu
    /// </summary>
    public void BackButton()
    {
        GameManager.Instance.players.Clear();

        mainMenu.SetActive(true);
        connectMenu.SetActive(false);
        lobbyMenu.SetActive(false);

        Client c = FindObjectOfType<Client>();
        if (c != null)
            Destroy(c.gameObject);

        
    }
   
    /// <summary>
    /// Function handling cancel button during connection atempt
    /// It destroys client and stops connection
    /// </summary>
    public void CancelConnection()
    {
        Client c = FindObjectOfType<Client>();
        
        if (c != null)
        {
            c.CloseConnection();
            Destroy(c.gameObject);
        }
            
    }

    /// <summary>
    /// Function that handles button for local game.
    /// It changes the scene to game scene without creating tcp client and starts the game
    /// </summary>
    public void LocalButton()
    {
        GameManager.Instance.StartGame();
    }

    /// <summary>
    /// Function that hides all submenuse except lobby
    /// </summary>
    public void GetBackToLobby()
    {
        lobbyMenu.SetActive(true);
        mainMenu.SetActive(false);
        connectMenu.SetActive(false);
    }

    /// <summary>
    /// Fills the lobby with information about connected players
    /// </summary>
    public void PrimeLobby()
    {
        PlayerList playerList = GameObject.Find("PlayerList").GetComponent<PlayerList>();
        playerList.Prime(GameManager.Instance.players);
    }

    /// <summary>
    /// Exits the application
    /// </summary>
    public void ExitButton()
    {
        GameManager.Instance.ExitApplication();
    }


}
