#include <vector>
#include "checkers.h"
#include <fstream>
#include <sstream>
#include <mutex>
#include <cstring>

using namespace std;

void Board::updatePieces(int fromRow, int fromCol, int toRow, int toCol) {
    pieces[toRow][toCol].setType(pieces[fromRow][fromCol].getType()); // Move the piece
    pieces[fromRow][fromCol].setType(Piece::NONE); // Clear the original position

}

void Board::initPieces() {
    pieces.resize(row, vector<Piece>(column, Piece(Piece::NONE)));
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < column; ++j) {
            if (board[i][j] == 0) {
                pieces[i][j] = Piece(Piece::BLACK);
            }
        }
    }

    for (int i = 5; i < 8; ++i) {
        for (int j = 0; j < column; ++j) {
            if (board[i][j] == 0) { // Only on dark squares
                pieces[i][j] = Piece(Piece::WHITE);
            }
        }
    }
}

void Board::printBoard() {
	interface();
        string currentState = boardToString();
	cout << "<div id='board' class='table-container' data-board-state='" << currentState << "'>";
    	cout << "<table class='game-board'>\n";
	//labels the top of the board
	cout << "<tr>";
    	for (char col = 'A'; col < 'A' + column; ++col) {
		cout << "<td style='font-weight: bold; background-color: white;'>" << col << "</td>";
    	}
	cout << "</tr>\n";

	for (int i = 0; i < row; ++i) {
		cout << "<tr>";
		for (int j = 0; j < column; ++j) {
			string color = (board[i][j] == 0) ? "DarkSlateGrey" : "Cornsilk";
			cout << "<td style='background-color: " << color << ";'>";
			if (color == "DarkSlateGrey") {
				if (pieces[i][j].getType() == Piece::BLACK) {
					cout << "<img class='draggable' src='BlackCircle.png' width='45' height='45' draggable='true'>";
				} else if (pieces[i][j].getType() == Piece::WHITE) {
					cout << "<img class='draggable' src='WhiteCircle.png' width='45' height='45' draggable='true'>";
				}
			}
			cout << "</td>";
		}
		cout << "<td style='font-weight: bold; background-color: white;'>" << (i + 1) << "</td>";
		cout << "</tr>\n";
	}
	cout << "</table></div>\n";
	cout << "<input type='hidden' id='currentBoardState' value='" << currentState << "'>\n";
}

void Board::updateBoard(int fromRow, int fromCol, int toRow, int toCol) {
    static mutex boardMutex;
    lock_guard<mutex> lock(boardMutex);
    
    if (!isValidPosition(fromRow, fromCol) || !isValidPosition(toRow, toCol)) {
        throw runtime_error("Invalid position");
    }
    
    auto oldState = pieces;
    bool isCapture = abs(toRow - fromRow) == 2;
    
    try {
        updatePieces(fromRow, fromCol, toRow, toCol);
        
        if (isCapture) {
            removeCapturedPiece(fromRow, fromCol, toRow, toCol);
        }
        
        recordMove(fromRow, fromCol, toRow, toCol, isCapture);
        
        updateGameState();
        toggleTurn();
        saveState();
    } catch (...) {
        pieces = oldState;
        throw;
    }
}

bool Board::valid_move(int fromRow, int fromCol, int toRow, int toCol) {
    if (fromRow < 0 || fromRow >= row || fromCol < 0 || fromCol >= column ||
        toRow < 0 || toRow >= row || toCol < 0 || toCol >= column) {
        return false;
    }

    if (pieces[fromRow][fromCol].getType() == Piece::NONE) {
        return false;
    }

    if (pieces[toRow][toCol].getType() != Piece::NONE) {
        return false;
    }

    if (abs(toRow - fromRow) != 1 || abs(toCol - fromCol) != 1) {
        return false;
    }

    if (pieces[fromRow][fromCol].getType() == Piece::BLACK && toRow <= fromRow){
        return false;
    }

    if (pieces[fromRow][fromCol].getType() == Piece::WHITE && toRow >= fromRow) {
        return false;
    }

    if (board[toRow][toCol] != 0) {
        return false;
    }

    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);
    
    if (rowDiff == 2 && colDiff == 2) {
        int midRow = (fromRow + toRow) / 2;
        int midCol = (fromCol + toCol) / 2;
        Piece::Type midPiece = pieces[midRow][midCol].getType();
        Piece::Type currentPiece = pieces[fromRow][fromCol].getType();
        
        if ((currentPiece == Piece::BLACK && midPiece == Piece::WHITE) ||
            (currentPiece == Piece::WHITE && midPiece == Piece::BLACK)) {
            pieces[midRow][midCol].setType(Piece::NONE);
            return true;
        }
        return false;
    }

    // Check for mandatory captures
    bool hasCapture = false;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            if (pieces[i][j].getType() == pieces[fromRow][fromCol].getType() &&
                canCapture(i, j)) {
                hasCapture = true;
                break;
            }
        }
        if (hasCapture) break;
    }

    // If there's a capture available, only allow capture moves
    if (hasCapture && abs(toRow - fromRow) != 2) {
        return false;
    }

    return (rowDiff == 1 && colDiff == 1);
}


void Board::make_move(int fromRow, int fromCol, int toRow, int toCol) {
    if (valid_move(fromRow, fromCol, toRow, toCol)) {
        updateBoard(fromRow, fromCol, toRow, toCol);
    }
}

string Board::boardToString() const {
    string result;
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < column; ++j) {
            if (!result.empty()) result += ",";
            if (pieces[i][j].getType() == Piece::NONE) result += "0";
            else if (pieces[i][j].getType() == Piece::BLACK) result += "1";
            else if (pieces[i][j].getType() == Piece::WHITE) result += "2";
        }
    }
    return result;
}

void Board::stringToBoard(const string& boardState) {
    if (boardState.empty()) {
        throw runtime_error("Empty board state");
    }

    // Create temporary board for validation
    vector<vector<Piece>> tempPieces(row, vector<Piece>(column, Piece(Piece::NONE)));
    
    istringstream ss(boardState);
    string token;
    int r = 0, c = 0;
    
    // Collect all tokens first
    vector<string> tokens;
    while (getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    
    // Use static_cast to compare sizes safely
    if (tokens.size() != static_cast<size_t>(row * column)) {
        throw runtime_error("Board state data does not match board dimensions");
    }
    
    // Reset stream
    ss.clear();
    ss.str(boardState);
    
    // Reset counters
    r = 0;
    c = 0;
    
    while (getline(ss, token, ',')) {
        if (r >= row || c >= column) {
            throw runtime_error("Board state data exceeds board dimensions");
        }
        
        try {
            int value = stoi(token);
            if (value < 0 || value > 2) {
                throw runtime_error("Invalid piece value in board state");
            }
            tempPieces[r][c] = Piece(static_cast<Piece::Type>(value));
        } catch (const invalid_argument&) {
            throw runtime_error("Invalid data format in board state");
        }
        
        c++;
        if (c >= column) {
            c = 0;
            r++;
        }
    }
    
    // Only update actual board after validation succeeds
    pieces = tempPieces;
}

bool Board::checkmate() {
    bool foundWhite = false;
    bool foundBlack = false;
    
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < column; ++j) {
            if (pieces[i][j].getType() == Piece::WHITE) foundWhite = true;
            if (pieces[i][j].getType() == Piece::BLACK) foundBlack = true;
            if (foundWhite && foundBlack) return false;
        }
    }
    return !foundWhite || !foundBlack;
}

void Board::interface() {
    cout << "<form id=\"moveForm\" onsubmit=\"submitMove(event); return false;\">\n";
    cout << "    <div class=\"selection\">\n";
    cout << "        From:\n";
    cout << "        <select name=\"fromCol\" required>\n";
    cout << "            <option value=\"0\">A</option>\n";
    cout << "            <option value=\"1\">B</option>\n";
    cout << "            <option value=\"2\">C</option>\n";
    cout << "            <option value=\"3\">D</option>\n";
    cout << "            <option value=\"4\">E</option>\n";
    cout << "            <option value=\"5\">F</option>\n";
    cout << "            <option value=\"6\">G</option>\n";
    cout << "            <option value=\"7\">H</option>\n";
    cout << "        </select>\n";
    cout << "        <select name=\"fromRow\" required>\n";
    cout << "            <option value=\"1\">1</option>\n";
    cout << "            <option value=\"2\">2</option>\n";
    cout << "            <option value=\"3\">3</option>\n";
    cout << "            <option value=\"4\">4</option>\n";
    cout << "            <option value=\"5\">5</option>\n";
    cout << "            <option value=\"6\">6</option>\n";
    cout << "            <option value=\"7\">7</option>\n";
    cout << "            <option value=\"8\">8</option>\n";
    cout << "        </select>\n";
    cout << "        To:\n";
    cout << "        <select name=\"toCol\" required>\n";
    cout << "            <option value=\"0\">A</option>\n";
    cout << "            <option value=\"1\">B</option>\n";
    cout << "            <option value=\"2\">C</option>\n";
    cout << "            <option value=\"3\">D</option>\n";
    cout << "            <option value=\"4\">E</option>\n";
    cout << "            <option value=\"5\">F</option>\n";
    cout << "            <option value=\"6\">G</option>\n";
    cout << "            <option value=\"7\">H</option>\n";
    cout << "        </select>\n";
    cout << "        <select name=\"toRow\" required>\n";
    cout << "            <option value=\"1\">1</option>\n";
    cout << "            <option value=\"2\">2</option>\n";
    cout << "            <option value=\"3\">3</option>\n";
    cout << "            <option value=\"4\">4</option>\n";
    cout << "            <option value=\"5\">5</option>\n";
    cout << "            <option value=\"6\">6</option>\n";
    cout << "            <option value=\"7\">7</option>\n";
    cout << "            <option value=\"8\">8</option>\n";
    cout << "        </select>\n";
    cout << "        <input type=\"submit\" value=\"Make Move\">\n";
    cout << "        <input type=\"hidden\" name=\"boardAsString\" id=\"boardAsString\">\n";
    cout << "    </div>\n";
    cout << "</form>\n";
}

bool Board::canCapture(int row, int col) const {
    if (!isValidPosition(row, col)) return false;
    
    Piece::Type currentType = pieces[row][col].getType();
    if (currentType == Piece::NONE) return false;
    
    // Check all possible capture directions
    const int directions[4][2] = {{2,2}, {2,-2}, {-2,2}, {-2,-2}};
    for (const auto& dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];
        if (!isValidPosition(newRow, newCol)) continue;
        
        int midRow = row + dir[0]/2;
        int midCol = col + dir[1]/2;
        Piece::Type midType = pieces[midRow][midCol].getType();
        
        if (pieces[newRow][newCol].getType() == Piece::NONE &&
            ((currentType == Piece::BLACK && midType == Piece::WHITE) ||
             (currentType == Piece::WHITE && midType == Piece::BLACK))) {
            return true;
        }
    }
    return false;
}

void Board::updateGameState() {
    // Check for checkmate first
    if (checkmate()) {
        bool foundWhite = false;
        bool foundBlack = false;
        
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < column; ++j) {
                if (pieces[i][j].getType() == Piece::WHITE) foundWhite = true;
                if (pieces[i][j].getType() == Piece::BLACK) foundBlack = true;
            }
        }
        
        currentState = !foundWhite ? BLACK_WIN : 
                      !foundBlack ? WHITE_WIN : DRAW;
        return;
    }
    
    // Check for stalemate (no valid moves available)
    bool hasValidMoves = false;
    Piece::Type currentPlayer = isWhiteTurn ? Piece::WHITE : Piece::BLACK;
    
    for (int i = 0; i < row && !hasValidMoves; ++i) {
        for (int j = 0; j < column && !hasValidMoves; ++j) {
            if (pieces[i][j].getType() == currentPlayer) {
                // Check all possible moves
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        if (valid_move(i, j, i + di, j + dj)) {
                            hasValidMoves = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if (!hasValidMoves) {
        currentState = DRAW;
        return;
    }
    
    currentState = ONGOING;
}

void Board::recordMove(int fromRow, int fromCol, int toRow, int toCol, bool wasCapture) {
    moveHistory.push_back({fromRow, fromCol, toRow, toCol, wasCapture});
}

bool Board::undoLastMove() {
    if (moveHistory.empty()) return false;
    
    Move lastMove = moveHistory.back();
    moveHistory.pop_back();
    
    // Restore pieces to their original positions
    pieces[lastMove.fromRow][lastMove.fromCol] = pieces[lastMove.toRow][lastMove.toCol];
    pieces[lastMove.toRow][lastMove.toCol] = Piece(Piece::NONE);
    
    if (lastMove.wasCapture) {
        // Restore captured piece
        int midRow = (lastMove.fromRow + lastMove.toRow) / 2;
        int midCol = (lastMove.fromCol + lastMove.toCol) / 2;
        pieces[midRow][midCol] = Piece(
            pieces[lastMove.fromRow][lastMove.fromCol].getType() == Piece::BLACK ? 
            Piece::WHITE : Piece::BLACK
        );
    }
    
    return true;
}

void Board::removeCapturedPiece(int fromRow, int fromCol, int toRow, int toCol) {
    int midRow = (fromRow + toRow) / 2;
    int midCol = (fromCol + toCol) / 2;
    pieces[midRow][midCol].setType(Piece::NONE);
}

void Board::saveState() const {
    const std::string filename = "game_state.txt";
    const std::string tempFile = filename + ".tmp";

    try {
        // Validate directory permissions with test write
        std::ofstream testWrite("test_write_permission.tmp");
        if (!testWrite) {
            throw std::runtime_error("Cannot write to current directory. Check permissions.");
        }
        testWrite.close();
        std::remove("test_write_permission.tmp");

        // Open the temporary file for writing
        std::ofstream file(tempFile, std::ios::out | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to create temporary file: " + std::string(std::strerror(errno)));
        }

        // Write the board state
        std::string boardStateToSave = boardToString();
        file << boardStateToSave << std::endl;

        // Check if the data was written successfully
        if (!file.good()) {
            throw std::runtime_error("Error writing to temporary file");
        }
        file.close();

        // Rename temporary file to final file atomically
        if (std::rename(tempFile.c_str(), filename.c_str()) != 0) {
            throw std::runtime_error("Error renaming temporary file: " + std::string(std::strerror(errno)));
        }
    } catch (const std::exception& e) {
        // Ensure cleanup of temporary files
        std::cerr << "Error saving game state: " << e.what() << std::endl;
        std::remove(tempFile.c_str());
        throw;
    }
}


bool Board::loadState() {
    ifstream file("game_state.txt", ios::in | ios::binary);
    if (!file) return false;
    
    string boardState;
    if (!getline(file, boardState)) return false;
    
    try {
        stringToBoard(boardState);
        return true;
    } catch (...) {
        return false;
    }
}

void Board::toggleTurn() {
    isWhiteTurn = !isWhiteTurn;
    // Add turn indicator to HTML output
    cout << "<div id='turnIndicator' class='" 
         << (isWhiteTurn ? "white-turn" : "black-turn") 
         << "'>Current Turn: " 
         << (isWhiteTurn ? "White" : "Black") 
         << "</div>\n";
}
