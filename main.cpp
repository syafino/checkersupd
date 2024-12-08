#include "checkers.h"
#include <iostream>

using namespace std;

int main() {
    try {
        cout << "Content-Type: text/html\r\n";
        cout << "Cache-Control: no-cache\r\n";
        cout << "X-Content-Type-Options: nosniff\r\n";
        cout << "X-Frame-Options: DENY\r\n";
        cout << "Content-Security-Policy: default-src 'self'; img-src 'self' data:; style-src 'self' 'unsafe-inline'\r\n";
        cout << "\r\n";
        cout << "<!DOCTYPE html>\n";
        cout << "<html>\n";
        cout << "<head>\n";
        cout << "<link rel=\"stylesheet\" href=\"styles.css\">\n";
        cout << "<script src='checkers.js'></script>\n";
        cout << "</head>\n";
        cout << "<body>\n";
        
        Board gameboard;
        gameboard.initPieces();
        gameboard.printBoard();
        
        cout << "</body>\n";
        cout << "</html>\n";
    } catch (const exception& e) {
        cout << "Content-Type: text/html\n\n";
        cout << "Error: " << e.what() << endl;
    }
    return 0;
}
