#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cstring>
#include <memory>
#include <vector>
#include "checkers.h"

using namespace std;

string getPostData() {
    const char* contentLengthEnv = getenv("CONTENT_LENGTH");
    if (!contentLengthEnv) {
        throw runtime_error("No Content-Length header");
    }

    size_t contentLength = stoul(contentLengthEnv);
    const size_t maxSize = MAX_CONTENT_LENGTH;

    if (contentLength > maxSize) {
        throw runtime_error("Input exceeds maximum allowed size");
    }

    // Use vector for dynamic memory allocation
    vector<char> buffer(contentLength + 1, 0);
    
    // Read exactly the number of bytes specified
    cin.read(buffer.data(), contentLength);
    size_t bytesRead = cin.gcount();

    if (bytesRead != contentLength) {
        throw runtime_error("Incomplete data read");
    }

    return string(buffer.data());
}

string getFormValue(const string& data, const string& key) {
    string keyStr = key + "=";
    size_t pos = data.find(keyStr);
    if (pos == string::npos) return "";
    
    pos += keyStr.length();
    size_t endPos = data.find('&', pos);
    if (endPos == string::npos) endPos = data.length();
    
    string value = data.substr(pos, endPos - pos);
    // URL decode the value
    string decoded;
    for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == '%' && i + 2 < value.length()) {
            string hex = value.substr(i + 1, 2);
            char ch = static_cast<char>(stoi(hex, nullptr, 16));
            decoded += ch;
            i += 2;
        } else if (value[i] == '+') {
            decoded += ' ';
        } else {
            decoded += value[i];
        }
    }
    return decoded;
}

string sanitizeInput(const string& input) {
    string result;
    for (char c : input) {
        if (isalnum(c) || c == ',' || c == '-' || c == '_') {
            result += c;
        }
    }
    if (result.length() > MAX_CONTENT_LENGTH) {
        throw runtime_error("Input too long");
    }
    return result;
}

int main() {
    try {
        cout << "Content-Type: text/html\r\n";
        cout << "Cache-Control: no-cache\r\n";
        cout << "X-Content-Type-Options: nosniff\r\n";
        cout << "X-Frame-Options: DENY\r\n";
        cout << "Content-Security-Policy: default-src 'self'; img-src 'self' data:; style-src 'self' 'unsafe-inline'\r\n";
        cout << "\r\n";
        
        // Capture POST data
        string postData = getPostData();
        
        // More verbose logging
        cerr << "Received POST data: [" << postData << "]" << endl;
        cerr << "Post data length: " << postData.length() << endl;

        // Initialize variables with detailed logging
        int fromRow = -1, fromCol = -1, toRow = -1, toCol = -1;
        string boardState;

        try {
            // Parse the move coordinates with more detailed error handling
            string fromRowStr = getFormValue(postData, "fromRow");
            string fromColStr = getFormValue(postData, "fromCol");
            string toRowStr = getFormValue(postData, "toRow");
            string toColStr = getFormValue(postData, "toCol");
            
            // Detailed logging added back
            cerr << "fromRow: " << fromRowStr << endl;
            cerr << "fromCol: " << fromColStr << endl;
            cerr << "toRow: " << toRowStr << endl;
            cerr << "toCol: " << toColStr << endl;

            boardState = sanitizeInput(getFormValue(postData, "boardAsString"));
            cerr << "boardState: " << boardState << endl;
	
            // Convert to integers if values exist
            if (!fromRowStr.empty()) fromRow = stoi(fromRowStr) - 1;
            if (!fromColStr.empty()) fromCol = stoi(fromColStr);
            if (!toRowStr.empty()) toRow = stoi(toRowStr) - 1;
            if (!toColStr.empty()) toCol = stoi(toColStr);

            // More detailed validation logging
            cerr << "Parsed coordinates - fromRow: " << fromRow 
                 << ", fromCol: " << fromCol 
                 << ", toRow: " << toRow 
                 << ", toCol: " << toCol << endl;

            // Add explicit range validation
            if (fromRow < 0 || fromRow >= BOARD_SIZE || 
                fromCol < 0 || fromCol >= BOARD_SIZE ||
                toRow < 0 || toRow >= BOARD_SIZE || 
                toCol < 0 || toCol >= BOARD_SIZE) {
                throw runtime_error("Move coordinates out of range");
            }

            Board gameBoard;
            if (!boardState.empty()) {
                gameBoard.stringToBoard(boardState);
                
                // Detailed move validation logging
                bool isValidMove = gameBoard.valid_move(fromRow, fromCol, toRow, toCol);
                cerr << "Is move valid: " << (isValidMove ? "Yes" : "No") << endl;
                
                if (isValidMove) {
                    gameBoard.updateBoard(fromRow, fromCol, toRow, toCol);
                } else {
                    throw runtime_error("Invalid move");
                }
            } else {
                cerr << "No board state - initializing new board" << endl;
                gameBoard.initPieces();
            }
            gameBoard.printBoard();
        }
        catch (const exception& e) {
            // Print a detailed error message
            cerr << "Error processing move: " << e.what() << endl;
            cout << "<div id='board'>Error processing move: " << e.what() << "</div>";
            cout << "<input type='hidden' id='currentBoardState' value=''>";
        }
    }
    catch (const exception& e) {
        cerr << "Global error processing request: " << e.what() << endl;
        cout << "<div id='board'>Global error processing request: " << e.what() << "</div>";
        cout << "<input type='hidden' id='currentBoardState' value=''>";
    }

    return 0;
}
