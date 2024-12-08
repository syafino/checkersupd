console.log("JavaScript loaded");

// Function to load the board initially
function loadBoard() {
    fetch('checkers.cgi')
        .then(response => {
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return response.text();
        })
        .then(data => {
 	    console.log('Server data:', data);
            const boardDiv = document.createElement('div');
            boardDiv.innerHTML = data;

            const board = document.getElementById('board');
            if (!board) throw new Error('Board element not found');

            // Update board content
            board.innerHTML = boardDiv.querySelector('#board')?.innerHTML || '';

            // Ensure form elements exist
            const boardAsString = document.getElementById('boardAsString');
            const currentBoardState = document.getElementById('currentBoardState');
            const boardState = boardDiv.querySelector('#board')?.getAttribute('data-board-state') || 
                               boardDiv.querySelector('#currentBoardState')?.value;
            
            if (boardState) {
                boardAsString.value = boardState;
                currentBoardState.value = boardState;
            } else {
                throw new Error('No board state found in server response');
            }
        })
        .catch(error => {
            console.error('Error:', error);
            document.getElementById('message').innerHTML = 
                `<div class="error">Error loading board: ${error.message}</div>`;
        });
}

// Function to handle form submission and move logic
function submitMove(event) {
    event.preventDefault();
    
    const form = document.getElementById('moveForm');
    if (!form) {
        console.error('Move form not found');
        return;
    }
    
    // Use URLSearchParams for proper encoding
    const formData = new URLSearchParams();
    for (let [key, value] of new FormData(form).entries()) {
        formData.append(key, value);
    }
    
    const submitButton = form.querySelector('button[type="submit"]');
    if (submitButton) submitButton.disabled = true;
    
    // Log the exact form data being sent
    console.log('Form Data:');
    for (let [key, value] of formData.entries()) {
        console.log(`${key}: ${value}`);
    }
    
    fetch('update_board.cgi', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: formData
    })
    .then(response => {
        console.log('Response status:', response.status);
        console.log('Response headers:', Object.fromEntries(response.headers.entries()));
        return response.text();
    })
    .then(data => {
        // Log the entire raw response
        console.log('Raw response data:', data);
        
        const parser = new DOMParser();
        const htmlDoc = parser.parseFromString(data, 'text/html');
        
        const newBoard = htmlDoc.getElementById('board');
        const newBoardState = htmlDoc.getElementById('currentBoardState');
        
        if (!newBoard || !newBoardState) {
            console.error('Parsed data:', data);
            throw new Error('Invalid server response');
        }
        
        document.getElementById('board').innerHTML = newBoard.innerHTML;
        document.getElementById('boardAsString').value = newBoardState.value;
        document.getElementById('currentBoardState').value = newBoardState.value;
        
        document.getElementById('message').innerHTML = 
            '<div class="success">Move completed successfully</div>';
    })
    .catch(error => {
        console.error('Full error:', error);
        document.getElementById('message').innerHTML = 
            `<div class="error">Error: ${error.message}</div>`;
    })
    .finally(() => {
        if (submitButton) submitButton.disabled = false;
    });
}

// Function to update the turn indicator
function updateTurnIndicator(isWhiteTurn) {
    const turnIndicator = document.getElementById('turnIndicator');
    turnIndicator.textContent = `Current Turn: ${isWhiteTurn ? 'White' : 'Black'}`;
    turnIndicator.className = isWhiteTurn ? 'white-turn' : 'black-turn';
}

// Initialize board on page load
window.onload = loadBoard;

