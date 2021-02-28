using System;
using System.Collections;
using UnityEngine;
using TMPro;
using UnityEngine.UI;
using Lean.Transition;

/// <summary>
/// Class OthelloBoard hold all the information and logic for the game Othello
/// </summary>
public class OthelloBoard : MonoBehaviour
{
    // Reference to the othello borad, OthelloBoard is singleton
    public static OthelloBoard Instance { set; get; }

    // 2D Array with the game grid
    public Chip[,] chips = new Chip[8, 8];

    // Reference to the score text for white player
    public TextMeshPro whiteScore;

    // Reference to the score text for black player
    public TextMeshPro blackScore;

    // Reference to game object containing all the dots representing legal moves
    private GameObject legalMoveDotsContainer;

    // Reference to the game object with dot prefab that represents legal move on the board
    public GameObject legalMoveDotPrefab;

    // Reference to the game object with chip prefab
    public GameObject chipPrefab;

    // Reference to the game object with highLight prefab
    public GameObject highLight;

    // Reference to the game object with particle effects that shows who is on the turn
    public GameObject particles;

    // Reference to the canvas group used to show alerts
    public CanvasGroup alertCanvas;

    // float representing the time from the last alert
    private float lastAlert;

    // indicates if the alert is active at the moment (shown/hidden)
    private bool alertActive;

    // array list with all legal moves in the current state of the game
    private ArrayList legalMoves;

    // array list with all dots that are representing possible legal moves on the board
    private ArrayList legalMovesDots = new ArrayList();

    // Offset of the board from the center of the Unity universe
    private Vector3 boardOffset = new Vector3(-4f, 0.08f, 4f);

    // Offset of each chip from the center of the game board
    private Vector3 chipOffset = new Vector3(-0.5f, 0, 0.5f);

    // Offset of the highlight from the center of the game board
    private Vector3 highlightOffset = new Vector3(0.5f, 0, -0.5f);

    // Vector representing current position of the mouse
    private Vector2 mouseOver;

    // Pomocné pole pro kontrolu všech smìrù (-1, -1; 0, -1; ... 1, 1) 
    protected static int[] DX = { -1, 0, 1, -1, 1, -1, 0, 1 };
    protected static int[] DY = { -1, -1, -1, 0, 0, 1, 1, 1 };

    // Indicates if the player is white or black
    public bool isWhite;

    // Indicates if it's whites or blacks turn
    public bool isWhiteTurn;

    // Indicates if the game is over
    private bool gameIsOver;

    // Float representing time since the game ended
    private float winTime;

    // Reference to the client for sending messages
    private Client client;


    /// <summary>
    /// Structure for position cordinates
    /// </summary>
    public struct Coordinate : IComparable<Coordinate>
    {
        public int X, Y;
        public Coordinate(int x)
        {
            X = x;
            Y = 0;
        }
        public Coordinate(int x, int y)
        {
            X = x;
            Y = y;
        }

        public int CompareTo(Coordinate other)
        {
            return X.CompareTo(other.X);
        }
    }


    // Start is called before the first frame update
    /// <summary>
    /// Generates start chips, initializes game varibales and visualises legal moves
    /// </summary>
    void Start()
    {
        Instance = this;
        client = FindObjectOfType<Client>();

        if (client)
        {
            isWhite = !client.isHost;
        }
        else
        {
            isWhite = false;
        }

        isWhiteTurn = false;

        legalMoveDotsContainer = new GameObject();
        legalMoveDotsContainer.name = "LegalMoves";

        GenerateStartChips();
        legalMoves = CalculateLegalMoves(chips, isWhiteTurn);
        VisualiseLegalMoves();

    }

    /// <summary>
    /// If the game is over waits 3seconds and sends players back to lobby
    /// or main menu (for local game)
    /// Draws the alert with the information send from the server on the screen
    /// Updates the position of the mouse and checks if the user clicked on the board
    /// </summary>
    // Update is called once per frame
    void Update()
    {

        if (gameIsOver)
        {
            if(Time.time - winTime > 3.0f)
            {
                if (client)
                {
                    GameManager.Instance.GetBackToLobby();
                }
                else
                {
                    GameManager.Instance.MainMenu();
                }
                
            }
            return;
        }
            

        UpdateMouseOver();
        UpdateAlert();

        // If it is my turn
        if ((isWhite)? isWhiteTurn : !isWhiteTurn)
        {
            int x = (int)mouseOver.x;
            int y = (int)mouseOver.y;

            if (Input.GetMouseButtonDown(0))
            {
                SelectPosition(x, y);
            }
        }
    }

    /// <summary>
    /// Generates the chips on the starting position
    /// </summary>
    private void GenerateStartChips()
    {
        GenerateChip(4, 4, true);
        GenerateChip(4, 5, false);
        GenerateChip(5, 4, false);
        GenerateChip(5, 5, true);
    }

    /// <summary>
    /// Instantiate the chip prefab and moves it to the position on the board given as the parameters
    /// </summary>
    /// <param name="x">x postion of the board</param>
    /// <param name="y">y posiion of the board</param>
    /// <param name="isWhite">indicates if the generated chip should be white or black</param>
    /// <returns>Returns the reference to the newly created chip</returns>
    private Chip GenerateChip(int x, int y, bool isWhite)
    {

        Chip chip;
        GameObject go = Instantiate(chipPrefab) as GameObject;
        go.transform.SetParent(transform);
        chip = go.GetComponent<Chip>();


        if (isWhite)
        {
            chip.isWhite = true;   
        }
        else
        {
            chip.isWhite = false;
        }
   

        chips[x-1, y-1] = chip;
        MoveChip(chip, x, y);



        return chip;

    }

    /// <summary>
    /// Moves the chip to the position on the board
    /// </summary>
    /// <param name="chip">Reference to the chip that should be moved</param>
    /// <param name="x">x postion of the board</param>
    /// <param name="y">y posiion of the board</param>
    private void MoveChip(Chip chip, int x, int y)
    {
        chip.transform.position = (Vector3.right * x) + (Vector3.back * y) + boardOffset + chipOffset;
        chip.updateDefPosition();
    }

    /// <summary>
    /// Updates the position of the mouse
    /// </summary>
    private void UpdateMouseOver()
    {
        // If their is camera - Safety check
        if (!Camera.main)
        {
            Debug.Log("Unable to find main camera");
            return;
        }

        RaycastHit hit;
        if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 25.0f, LayerMask.GetMask("Board")))
        {
            mouseOver.x = (int)(hit.point.x - boardOffset.x);
            mouseOver.y = 7 - (int)(hit.point.z + boardOffset.z);
            highLight.transform.position = (Vector3.right * mouseOver.x) + (Vector3.back * mouseOver.y) + boardOffset + highlightOffset;
        }
        else
        {
            mouseOver.x = -1;
            mouseOver.y = -1;
        }
    }

    /// <summary>
    /// Selects the position on the game board, validates it and makes a move (one round)
    /// Sends the move to the server
    /// </summary>
    /// <param name="x">x postion of the board</param>
    /// <param name="y">y posiion of the board</param>
    private void SelectPosition(int x, int y)
    {
        if (!InBounds(x, y) || chips[x, y] != null) return;
        else
        {
            Coordinate cord = new Coordinate(x, y);
            if (IsLegalMove(chips, cord, isWhiteTurn))
            {
                MakeMove(x, y, isWhiteTurn);
                if (client)
                {
                    string msg = "CMOV~";
                    msg += x + "~";
                    msg += y;

                    client.Send(msg);
                }
                
            }
            else
                return;
                
        }
    }

    /// <summary>
    /// Checks if the move on the coordinates (x, y) is a valid one for the white or black player
    /// </summary>
    /// <param name="board">2D array with the current state of the game</param>
    /// <param name="cord">Coordinates (x,y) of the move</param>
    /// <param name="isWhite">Whether the player is white or black</param>
    /// <returns>True if the move is valid, otherwise false</returns>
    public bool IsLegalMove(Chip[,] board, Coordinate cord, bool isWhite)
    {
        // Pokud je mimo Hru nebo pokud již obsahuje žeton -> neplatný pohyb
        if (!InBounds(cord.X, cord.Y) || chips[cord.X, cord.Y] != null) return false;

        for (int i = 0; i < DX.Length; i++)
        {
            bool sawOther = false;
            int x = cord.X, y = cord.Y;
            for (int j = 0; j < DY.Length; j++)
            {
                x += DX[i];
                y += DY[i];
                if (!InBounds(x, y)) break;
                Chip chip = board[x, y];
                if (chip == null) break;
                else if (chip.isWhite != isWhite) sawOther = true;
                else if (sawOther) return true;
                else break;
            }
        }

        return false;
    }

    /// <summary>
    /// Calculates all the possible legal moves in current state of the game for the white or black player
    /// </summary>
    /// <param name="board">2D array with the current state of the game</param>
    /// <param name="isWhite">Whether the player is white or black</param>
    /// <returns></returns>
    public ArrayList CalculateLegalMoves(Chip[,] board, bool isWhite)
    {
        ArrayList moves = new ArrayList();

        for (int y = 0; y < board.GetLength(0); y++)
        {
            for (int x = 0; x < board.GetLength(1); x++)
            {
                Coordinate cord = new Coordinate(x, y);
                if (board[cord.X, cord.Y] != null) continue;
                if (IsLegalMove(board, cord, isWhite)) moves.Add(cord);
            }
        }

        return moves;
    }

    /// <summary>
    /// Makes one move (one round)
    /// Generates chip on the position, flips all the other chips according to the game rules, changes current player on the move,
    /// calculates score and visualizes all new possible moves.
    /// It also checks whether the game ends.
    /// Before using this function it should already be checked if the params passed are of a legal move
    /// </summary>
    /// <param name="x">x postion of the board</param>
    /// <param name="y">y postion of the board</param>
    /// <param name="isWhite">Whether the player is white or black</param>
    public void MakeMove(int x, int y, bool isWhite)
    {
        ArrayList toFlip = new ArrayList();

        GenerateChip(x+1, y+1, isWhite);

        for (int i = 0; i < DX.Length; i++)
        {
            int tmpx = x, tmpy = y;
            for (int j = 0; j < chips.GetLength(0); j++)
            {
                tmpx += DX[i];
                tmpy += DY[i];

                if (!InBounds(tmpx, tmpy)) break;
                Coordinate cord = new Coordinate(tmpx, tmpy);
                Chip chip = chips[tmpx, tmpy];
                if (chip == null) break;
                else if (chip.isWhite != isWhite) toFlip.Add(cord);
                else
                {
                    foreach (Coordinate crd in toFlip) chips[crd.X, crd.Y].flip();
                    break;
                }
            }
            toFlip.Clear();
        }

        isWhiteTurn = !isWhiteTurn;
        if (!client) this.isWhite = !this.isWhite;
        CalculateScore();
        legalMoves = CalculateLegalMoves(chips, isWhiteTurn);
        VisualiseLegalMoves();

        if (legalMoves.Count == 0)
        {
            string message = isWhiteTurn ? "Bílý nemìl žádný platný tah a byl pøeskoèen" : "Èerný nemìl žádný platný tah a byl pøeskoèen";
            Alert(message);
            isWhiteTurn = !isWhiteTurn;
            if (!client) this.isWhite = !this.isWhite;
            legalMoves = CalculateLegalMoves(chips, isWhiteTurn);
            VisualiseLegalMoves();

            if (legalMoves.Count == 0)
            {
                EndGame();
                return;
            }

        }

    }

    /// <summary>
    /// Visualizes all the currently legal moves on the board with the dots
    /// </summary>
    private void VisualiseLegalMoves()
    {
        foreach (GameObject go in legalMovesDots)
        {
            Destroy(go);
        }

        foreach (Coordinate cord in legalMoves)
        {
            GameObject go = Instantiate(legalMoveDotPrefab);
            go.transform.SetParent(legalMoveDotsContainer.transform);
            go.transform.position = (Vector3.right * cord.X) + (Vector3.back * cord.Y) + boardOffset + highlightOffset;
            legalMovesDots.Add(go);
        }

        if (isWhiteTurn)
        {
            particles.transform.localPositionTransition(new Vector3(-9, 0, 1), 0.3f);
        }
        else
        {
            particles.transform.localPositionTransition(new Vector3(-9, 0, -1), 0.3f);
        }
    }
 
    /// <summary>
    /// Calculates the score for both players by calculating all the chips of their colors currently on the board
    /// It also checks if the score adds up to 64. If so, game ended
    /// </summary>
    public void CalculateScore()
    {
        int whiteS = 0;
        int blackS = 0;

        foreach(Chip chip in chips)
        {
            if (chip == null) continue;
            if (chip.isWhite) whiteS++;
            else blackS++;
        }

        whiteScore.text = whiteS.ToString();
        blackScore.text = blackS.ToString();

        if ((whiteS + blackS) == 64)
        {

            EndGame();
        }
    }

    /// <summary>
    /// Resets the game to the start state
    /// </summary>
    public void ResetGame()
    {
        ResetBoard();

        GenerateStartChips();

        whiteScore.text = "2";
        blackScore.text = "2";

        isWhiteTurn = false;
        legalMoves = CalculateLegalMoves(chips, isWhiteTurn);
        VisualiseLegalMoves();
    }

    /// <summary>
    /// Clears the entire board from the chips
    /// </summary>
    public void ResetBoard()
    {
        foreach (Transform child in transform)
        {
            Destroy(child.gameObject);
        }

        Array.Clear(chips, 0, chips.Length);
    }

    /// <summary>
    /// Show an alert with the message given as argument
    /// </summary>
    /// <param name="message">Message that should be shown in the alert</param>
    public void Alert(string message)
    {
        alertCanvas.GetComponentInChildren<Text>().text = message;
        lastAlert = Time.time;
        alertActive = true;
        alertCanvas.alpha = 1;
    }

    /// <summary>
    /// Updates the timing of the alert (alert should be visible only for 3s, then it slowly fades away)
    /// </summary>
    public void UpdateAlert()
    {
        if (alertActive)
        {
            if(Time.time - lastAlert > 1.5f)
            {

                alertCanvas.alpha = 1 - ((Time.time - lastAlert) - 1.5f);

                if(Time.time - lastAlert > 2.5f)
                {
                    alertActive = false;
                }
            }
        }
    }


    /// <summary>
    /// Ends the game and annoucnes winner
    /// </summary>
    public void EndGame()
    {

        winTime = Time.time;
        gameIsOver = true;
        
        if (Int16.Parse(whiteScore.text) > Int16.Parse(blackScore.text))
        {
            Alert("Bílý vyhrál!");
        }
        else if (Int16.Parse(blackScore.text) > Int16.Parse(whiteScore.text))
        {
            Alert("Èerný vyhrál!");
        }
        else
        {
            Alert("Remíza!");
        }

    }

    /// <summary>
    /// Recovers the game to the state given as parameter
    /// </summary>
    /// <param name="gameState">String with the state of the game</param>
    /// <param name="whitesMove">Bool representing current player on the move (true if white is on the move, false if black is on the move)</param>
    public void RecoverGame(string gameState, bool whitesMove)
    {
        int i = 0;
        int j = 0;
        ResetBoard();
        
        foreach(char c in gameState)
        {

            int val = c - '0';
            if (val != 0)
            {
                
                {
                    if (val == 1)
                    {
                        GenerateChip(i+1, j+1, false);
                    }
                    else
                    {
                        GenerateChip(i+1, j+1, true);
                    }
                }

            }


            i++;
            i = i % 8;
            if (i == 0)
            {
                j++;
            }

            
        }
        this.isWhite = !client.isHost;
        this.isWhiteTurn = whitesMove;
        CalculateScore();
        legalMoves = CalculateLegalMoves(chips, isWhiteTurn);
        VisualiseLegalMoves();
    }

    /// <summary>
    /// Checks if the coordinates given as parameters are in the bounds of the board
    /// </summary>
    /// <param name="x">x position on the board</param>
    /// <param name="y">y position on the board</param>
    /// <returns>True if the x and y positions are within the board bounds, otherwise false</returns>
    private bool InBounds( int x, int y )
    {
        return (x >= 0) && (x < chips.GetLength(0)) && (y >= 0) && (y < chips.GetLength(1));
    }

    /// <summary>
    /// Activates highlight upon mouse entering the board object
    /// </summary>
    private void OnMouseEnter()
    {
        highLight.SetActive(true);
    }

    /// <summary>
    /// Deactivates highlight upon mouse exiting the board object
    /// </summary>
    private void OnMouseExit()
    {
        highLight.SetActive(false);
    }
}
