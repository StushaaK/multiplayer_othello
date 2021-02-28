#include "Othello.h"

  const int DX[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
  const int DY[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

Othello::Othello(std::string player1, std::string player2, Server *server)
{
    this->player1 = player1;
    this->player2 = player2;
    this->server = server;

    whites_move = false;
    

    //vyresetuj hru
    memset(board, kFree, sizeof(board));
    generateStartChips();

}

Othello::~Othello()
{
    sleep(1);
}

Othello::GameState Othello::makeMove(int x, int y)
{
    std::vector<std::pair<int, int>> toFlip;
    std::vector<std::pair<int, int>> legalMoves;
    Chip forColor = (this->whites_move) ? kWhite : kBlack;

    

    
    board[x][y] = forColor;

    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        int tmpX = x;
        int tmpY = y;
        
        for (int j = 0; j < GAME_BOARD_SIZE; j++)
        {
            tmpX += DX[i];
            tmpY += DY[i];
            

            if (!isInBounds(tmpX, tmpY)) break;
            Chip chip = board[tmpX][tmpY];
            if (chip == kFree) break;
            else if (chip != forColor) toFlip.push_back({tmpX, tmpY});
            else
            {
                for (auto it : toFlip)
                {
                    board[it.first][it.second] = forColor;
                }
                break;
            }
        }
        toFlip.clear();
    }


    (this->whites_move) ? server->sendMessage(player1,"SMOV~"+std::to_string(x) +"~"+ std::to_string(y)) : server->sendMessage(player2,"SMOV~"+std::to_string(x) +"~"+ std::to_string(y));
    this->whites_move = !this->whites_move;
    legalMoves = calculateLegalMoves(this->whites_move);
    

    if (legalMoves.empty())
    {
        this->whites_move = !this->whites_move;
        legalMoves = calculateLegalMoves(this->whites_move);

        if(legalMoves.empty())
        {
            return kPlayerOneWon;
        }
    }
    
    
    return kInProgress; 
}

void Othello::printBoard()
{
    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        for (int j = 0; j < GAME_BOARD_SIZE; j++)
        {
            std::cout << board[j][i] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Othello::generateStartChips()
{
    board[3][3] = kWhite;
    board[3][4] = kBlack;
    board[4][3] = kBlack;
    board[4][4] = kWhite;
}

bool Othello::isInBounds(int x, int y)
{
    return (x>=0) && (x < GAME_BOARD_SIZE) && (y >= 0) && (y < GAME_BOARD_SIZE);
}

bool Othello::isLegalMove(int x, int y, bool isWhite)
{
    
    if (!isInBounds(x, y) || board[x][y] != kFree) return false;



    Chip forColor = (isWhite) ? kWhite : kBlack;
   
    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        bool sawOther = false;
        int tmpX = x;
        int tmpY = y;
        
        for (int j = 0; j < GAME_BOARD_SIZE; j++)
        {
            
            tmpX += DX[i];
            tmpY += DY[i];

            
            if (!isInBounds(tmpX, tmpY))
            {
                break;
            }
            Chip chip = board[tmpX][tmpY];
            if (chip == kFree) break;
            else if (chip != forColor) sawOther = true;
            else if (sawOther) return true;
            else break;
        }
    }

    return false;
}

std::vector<std::pair<int, int>> Othello::calculateLegalMoves(bool isWhite)
{
    std::vector<std::pair<int, int>> moves;
    for (int y = 0; y < GAME_BOARD_SIZE; y++)
    {
        for (int x = 0; x < GAME_BOARD_SIZE; x++)
        {
            if(board[x][y] != kFree) continue;
            if(isLegalMove(x, y, isWhite)) moves.push_back({x, y});

        }
    }

    return moves;
}

std::string Othello::getGameState(std::string playerName) const
{
    std::string gameState = "";
    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        for (int j = 0; j < GAME_BOARD_SIZE; j++)
        {
            gameState.append(std::to_string(board[j][i]));
        }
    }

    gameState.append("~"+std::to_string(whites_move));
    
    if (player1 == playerName) gameState.append("~1");
    else gameState.append("~0");

    return gameState;
}

