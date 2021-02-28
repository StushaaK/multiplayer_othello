using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Lean.Transition;

/// <summary>
/// Class keeping all the information about lobby and functions to operates it
/// </summary>
public class Lobby : MonoBehaviour

{   // Reference to the gameObject with list of players
    public GameObject playerList;

    // Reference to the gameObject popUp window
    public GameObject popPup;

    // Refenrece to the gameObject with waiting window
    public GameObject waitingPanel;

    // Username of the opponent
    private string oponentName;


    /// <summary>
    /// Opens popup window with game request
    /// </summary>
    /// <param name="oponentName">Username of the opponent who sent the game request</param>
    public void OpenPopupWindow(string oponentName)
    {

        PopUp(popPup);
        Text popupText = GameObject.Find("Alert-Text").GetComponent<Text>();
        popupText.text = "You got a game invite from '"+ oponentName +"'!";
        this.oponentName = oponentName;
            
    }

    /// <summary>
    /// Opens the waiting panel with the text informing player about waiting for response
    /// </summary>
    /// <param name="playerName">Username of the opponent who did player sent the game request to</param>
    public void openWaitForOtherPlayer(string playerName)
    {
        PopUp(waitingPanel);
        this.oponentName = playerName;
    }

    /// <summary>
    /// Cancels sent game request and informs server about it
    /// </summary>
    public void cancelRequest()
    {
        Client c = FindObjectOfType<Client>();
        c.Send("CREQC~" + this.oponentName);
    }

    /// <summary>
    /// Closes the waiting panel
    /// </summary>
    public void closeWaitForOtherPlayer()
    {
        ClosePopUp(waitingPanel);
    }

    /// <summary>
    /// Closes the request window
    /// </summary>
    public void closeRequestWindow()
    {
        ClosePopUp(popPup);
    }

    /// <summary>
    /// Declines the game request and informs server about it
    /// </summary>
    public void DeclinePopUp()
    {
        Client c = FindObjectOfType<Client>();
        c.Send("CRES~" + this.oponentName + "~DECLINE");
        ClosePopUp(popPup);
    }

    /// <summary>
    /// Acceps the game request and informs server about it
    /// </summary>
    public void AcceptPopUp()
    {
        Client c = FindObjectOfType<Client>();
        c.isHost = false;
        c.Send("CRES~" + this.oponentName + "~ACCEPT");
        ClosePopUp(popPup);
    }

    /// <summary>
    /// Closes the pop up window given as parameter
    /// </summary>
    /// <param name="window">Popup window that should be closed</param>
    public void ClosePopUp(GameObject window)
    {
        window.SetActive(false);
        window.transform.localScale = new Vector3(0, 0, 0);
        GameObject.Find("LobbyList").GetComponent<CanvasGroup>().interactable = true;
    }

    /// <summary>
    /// Animates the pop up window given as parameter
    /// </summary>
    /// <param name="window">Window that should "pop up"</param>
    public void PopUp(GameObject window)
    {
        window.SetActive(true);
        window.transform.localScaleTransition(new Vector3(1, 1, 1), 0.3f);
        GameObject.Find("LobbyList").GetComponent<CanvasGroup>().interactable = false;
    }

    
}
