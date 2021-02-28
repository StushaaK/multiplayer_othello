using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

/// <summary>
/// Class PlayerList is representing the lobbys player list and is keeping all the references and information related to player list
/// </summary>
public class PlayerList : MonoBehaviour
{

    // Reference to the game object with player list item prefab
    public GameObject playerListItemPrefab;


    /// <summary>
    /// Adds player to the playe list and sets it up
    /// </summary>
    /// <param name="playerName">Username of the player that should be added</param>
    /// <param name="state">State of the player that is beeing added</param>
    public void AddPlayer(string playerName, PlayerListItem.PlayerState state)
    {
        GameObject go = Instantiate(playerListItemPrefab);
        go.transform.SetParent(transform, false);
        PlayerListItem player = go.GetComponent<PlayerListItem>();
        player.playerName = playerName;
        player.state = state;
        player.SetUp();
    }

    /// <summary>
    /// Deletes the player from the player list
    /// </summary>
    /// <param name="playerName">Username of the player that should be deleted</param>
    public void DeletePlayer(string playerName)
    {
        foreach (Transform child in transform)
        {
            PlayerListItem player = child.gameObject.GetComponent<PlayerListItem>();
            if (player.playerName == playerName)
            {
                Destroy(child.gameObject);
                break;
            }
        }
    }

    /// <summary>
    /// Completely clears the player list (deletes all players from lobby)
    /// </summary>
    public void ClearPlayerList()
    {
        foreach (Transform child in transform)
        {
              Destroy(child.gameObject);
              break;
        }
    }

    /// <summary>
    /// Update players state given as parameter
    /// </summary>
    /// <param name="playerName">Username of the player that should be updated in the list</param>
    /// <param name="state">New state of the player</param>
    public void UpdatePlayer(string playerName, PlayerListItem.PlayerState state)
    {
        foreach (Transform child in transform)
        {
            PlayerListItem player = child.gameObject.GetComponent<PlayerListItem>();
            if (player.playerName == playerName)
            {
                player.state = state;
                player.SetUp();
                break;
            }
        }
    }

    /// <summary>
    /// Primes the lobby by filling up the player list
    /// </summary>
    /// <param name="players">Dictionary with all the players connected to the server</param>
    public void Prime(Dictionary<string, PlayerListItem.PlayerState> players)
    {
        foreach(KeyValuePair<string, PlayerListItem.PlayerState> kvp in players)
        {
            GameObject go = Instantiate(playerListItemPrefab);
            go.transform.SetParent(transform, false);
            PlayerListItem player = go.GetComponent<PlayerListItem>();
            player.playerName = kvp.Key;
            player.state = kvp.Value;
            player.SetUp();
        }
    }

}
