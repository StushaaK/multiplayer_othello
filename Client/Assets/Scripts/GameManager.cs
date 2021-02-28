using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

/// <summary>
/// Class game manager is simpleton that keeps information about all connected player as well as manages the scenes of the game
/// </summary>
public class GameManager : MonoBehaviour
{

    // Dictionary of all players, key - player name, value - player state
    public Dictionary<string, PlayerListItem.PlayerState> players = new Dictionary<string, PlayerListItem.PlayerState>();

    // instance of the Game manager - singleton
    static GameManager _instance;

    // Queue of all the jobs that the game manager needs to do
    Queue<Action> jobs = new Queue<Action>();

    // Getter of the instance - singleton GameManager
    public static GameManager Instance { 
        get
        {
            if(_instance == null)
            {
                GameObject manager = new GameObject("[GameManager]");
                _instance = manager.AddComponent<GameManager>();
                DontDestroyOnLoad(manager);
            }
            return _instance;
        }
    
    }

    // Update is beeing called every frame
    // Game Manager checks the queue of the jobs and tries to invoke the next job
    public void Update()
    {
        while (jobs.Count > 0)
        {
            jobs.Dequeue().Invoke();
        }
    }

    /// <summary>
    /// Adds the job to the queue of jobs
    /// </summary>
    /// <param name="newJob">New job that should be added to the queue</param>
    internal void AddJob(Action newJob)
    {
        jobs.Enqueue(newJob);
    }

    /// <summary>
    /// Shuts down the whole application
    /// </summary>
    public void ExitApplication()
    {
        Application.Quit();
    }


    /// <summary>
    /// Starts the game by changing the scene from menu to game
    /// </summary>
    public void StartGame()
    {
        SceneManager.LoadScene("game");
    }

    /// <summary>
    /// Recovers the game
    /// </summary>
    /// <param name="state">String representing the state of the board to be recovered</param>
    /// <param name="isWhiteMove">Bool represeting who is on the move (true if it's whites move, false if it's blacks move)</param>
    /// <param name="isHost">Bool representing if the person whos game is beeing recovered was host (true if the client was host, otherwise false)</param>
    public void RecoverGame(string state, bool isWhiteMove, bool isHost)
    {
        OthelloBoard game = GameObject.Find("Board").GetComponent<OthelloBoard>();
        Client c = FindObjectOfType<Client>();
        c.isHost = isHost;
        game.RecoverGame(state, isWhiteMove);
    }

    /// <summary>
    /// Ends the current game
    /// </summary>
    public void EndGame()
    {
        OthelloBoard game = GameObject.Find("Board").GetComponent<OthelloBoard>();
        game.EndGame();
    }

    /// <summary>
    /// Changes the scene to menu
    /// </summary>
    public void MainMenu()
    {
        SceneManager.LoadScene("menu");
    }

    /// <summary>
    /// Displays the lobby menu
    /// </summary>
    public void DisplayLoby()
    {
        MenuController menuController = GameObject.Find("MenuController").GetComponent<MenuController>();
        menuController.connectMenu.SetActive(false);
        menuController.lobbyMenu.SetActive(true);
    }


    /// <summary>
    /// Starts the coroutine that goes back to connection menu
    /// </summary>
    public void GetBackToConnectionScreen()
    {
        StartCoroutine(GetBackToConnectionScreen_Courutine());
    }

    /// <summary>
    /// Starts the courutine that goes back to lobby
    /// </summary>
    public void GetBackToLobby()
    {
        StartCoroutine(GetBackToLobby_Coroutine());
    }

    /// <summary>
    /// Displays the alert with the title and text given as parameters
    /// </summary>
    /// <param name="title">Title of the alert message</param>
    /// <param name="text">Main text of the alert</param>
    public void displayConnectingScreenAlert(string title, string text)
    {
        MenuController menuController = GameObject.Find("MenuController").GetComponent<MenuController>();
        menuController.displaySocketError(title, text);
        //menuController.waitingPanel.SetActive(false);
    }

    /// <summary>
    /// Courutine that brings client back to connection screen
    /// </summary>
    /// <returns>null before the menu screen is loaded</returns>
    IEnumerator GetBackToConnectionScreen_Courutine()
    {
        SceneManager.LoadScene("menu");
        yield return null;
        MenuController menuController = GameObject.Find("MenuController").GetComponent<MenuController>();
        menuController.getBackToConnectionScreenWithError();
    }

    /// <summary>
    /// Courutine that brings client back to lobby
    /// </summary>
    /// <returns>null before the menu screen is loaded</returns>
    IEnumerator GetBackToLobby_Coroutine()
    {
        SceneManager.LoadScene("menu");
        yield return null;
        MenuController menuController = GameObject.Find("MenuController").GetComponent<MenuController>();
        menuController.GetBackToLobby();
        menuController.PrimeLobby();
    }





}
