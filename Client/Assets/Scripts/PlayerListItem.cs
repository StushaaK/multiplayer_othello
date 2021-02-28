using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

/// <summary>
/// Class PlayerListItem represents one Item inside lobbys player list
/// </summary>
public class PlayerListItem : MonoBehaviour
{
    // Name of the player
    public string playerName;

    // State of the player (BUSY/READY)
    public PlayerState state;

    // Enum with states of the player
    public enum PlayerState
    {
        kReady,
        kBusy,
    }

    /// <summary>
    /// Primes the text of the player list item and sets the color of circle indicating players state
    /// </summary>
    public void SetUp()
    {
        
        Text text = gameObject.GetComponentInChildren<Text>();
        Image status = gameObject.transform.Find("Status").GetComponent<Image>();

        text.text = playerName;
        
        switch(state)
        {
            case PlayerState.kBusy:
                status.color = new Color(255, 0, 0);
                break;
            case PlayerState.kReady:
                status.color = new Color(0, 0, 255);
                break;
            default:
                Debug.Log("Nothing");
                break;
        }

    }

    /// <summary>
    /// Method that is called upon clicking on player list item
    /// It's send game request to the server
    /// </summary>
    public void OnClick()
    {
        if (state != PlayerState.kBusy)
        {
            Client c = FindObjectOfType<Client>();
            c.isHost = true;
            c.Send("CREQ~" + playerName);
            Lobby lobby = GameObject.Find("Lobby").GetComponent<Lobby>();
            lobby.openWaitForOtherPlayer(playerName);
        }

    }
}
