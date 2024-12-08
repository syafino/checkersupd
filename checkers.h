#ifndef _CHECKERS_H_
#define _CHECKERS_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <mutex>  // Add for thread safety
#include <memory> // Add for smart pointers

using namespace std;

// Add these constants
#define MAX_CONTENT_LENGTH 4096
#define BOARD_SIZE 8

// Add before the Board class
enum GameState { ONGOING, WHITE_WIN, BLACK_WIN, DRAW };

class Piece{
	public:
        enum Type { NONE, BLACK, WHITE };
        Piece(Type type_= NONE): type(type_){}
        Type getType() const{return type;}
        void setType(Type type_){this->type = type_;}
        private:
        Type type;
        };


class Board{
    	private:
    	vector<vector<int> > board;
	vector<vector<Piece>> pieces;
    	int row;
    	int column;
	void updatePieces(int fromRow, int fromCol, int toRow, int toCol);

	struct Move {
		int fromRow, fromCol, toRow, toCol;
		bool wasCapture;
	};
	vector<Move> moveHistory;

	GameState currentState = ONGOING;  // Add this member
	bool isWhiteTurn = true;  // Add this member

	public:
    	Board() : row(BOARD_SIZE), column(BOARD_SIZE), currentState(ONGOING), isWhiteTurn(true) {
            board.resize(row, vector<int>(column, 0));  // Initialize with zeros
            pieces.resize(row, vector<Piece>(column, Piece(Piece::NONE)));
            for (int i = 0; i < row; ++i) {
                for (int j = 0; j < column; ++j) {
                    board[i][j] = (i + j) % 2;  // Set checkerboard pattern
                }
            }
        }
	void initPieces();
    	void printBoard();
    	void start(); //starts up the positions of the checkers objects.
    	bool valid_move(int fromRow, int fromCol, int toRow, int toCol); //checks if the move is valid
    	void make_move(int fromRow, int fromCol, int toRow, int toCol);  //makes the move
	void updateBoard(int fromRow, int fromCol, int toRow, int toCol);
	bool loadState();
	void saveState() const;
	void stringToBoard(const string& boardState);//for hidden html
	string boardToString() const; //for hidden html

	bool checkmate();
	void interface();

    bool isValidPosition(int row, int col) const {
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
    }
    
    bool canCapture(int row, int col) const;
    void removeCapturedPiece(int fromRow, int fromCol, int toRow, int toCol);

    bool isGameOver() const { return currentState != ONGOING; }
    GameState getGameState() const { return currentState; }
    void updateGameState();

	void recordMove(int fromRow, int fromCol, int toRow, int toCol, bool wasCapture);
	bool undoLastMove();
	bool isWhiteMove() const { return isWhiteTurn; }
	void toggleTurn();
};


#endif
