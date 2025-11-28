#include <iostream>
#include <SFML/Graphics.hpp>
#include <fstream>
#include "Tile.h"
#include <algorithm>
#include <random>
#include <chrono>


using namespace std;

//func to convert first letter of name to uppercase
string toUppercase(const string& name) {
    string result = name;
    for (int i = 0; i < result.length(); i++) {
        char& c = result[i];
        if (i == 0) {
            c = toupper(c);
        }
        else {
            c = tolower(c);
        }
    }
    return result;
}

// func to center text
void setText(sf::Text& text, float x, float y){
    sf::FloatRect textRect = text.getLocalBounds();
    sf::Vector2f newOrigin(textRect.position.x + textRect.size.x/2.0f, textRect.position.y + textRect.size.y/2.0f);
    text.setOrigin(newOrigin);
    text.setPosition(sf::Vector2f(x, y));
}


void randomize(vector<Tile> &tiles, int cols, int rows) {
    // randomize vector of tiles
    std::random_device rd;
    std::mt19937 gen(rd());
    shuffle(tiles.begin(), tiles.end(), gen);

    // set position of each tile
    sf::Texture hidden("../../files/images/tile_hidden.png");
    float tileWidth = hidden.getSize().x;
    float tileHeight = hidden.getSize().y;

    for (int i = 0; i < tiles.size(); i++) {
        tiles[i].setPosition((i % cols) * (tileWidth), (i / cols) * (tileHeight));
    }

    // iterate through array of pointers to tiles and set each tile's neighbors
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i].checkMine()) {
            continue;
        }

        vector<Tile*> neighbors;
        int currCol = (i % cols);
        int currRow = (i / cols);
        // find adjacent tiles

        int up = (i - cols);
        int down = (i + cols);
        int right = (i + 1);
        int left = (i - 1);

        // checks 3 above
        if (currRow > 0){ // checks if there is a row above
            neighbors.push_back(&tiles[up]);

            if (currCol > 0) {
                neighbors.push_back(&tiles[up - 1]); // up left
            }

            if (currCol < cols - 1) {
                neighbors.push_back(&tiles[up + 1]); // up right
            }
        }

        // checks three below
        if (currRow < rows - 1) { // checks if there is a row below
            neighbors.push_back(&tiles[down]);

            if (currCol > 0) {
                neighbors.push_back(&tiles[down - 1]); // down left
            }

            if (currCol < cols - 1) {
                neighbors.push_back(&tiles[down + 1]); // down right
            }
        }

        // checks left and right
        if (currCol > 0) { // checks left
            neighbors.push_back(&tiles[left]);
        }

        if (currCol < cols - 1) { // checks right
            neighbors.push_back(&tiles[right]);
        }

        tiles[i].setAdjacent(neighbors);
        tiles[i].setVal();
    }
}




int main() {
    //load config file
    ifstream file("../../files/config.cfg");
    if (!(file.is_open())) {
        cout << "Could not open file" << endl;
    }

    int cols;
    int rows;
    int mines;
    file >> cols;
    file >> rows;
    file >> mines;
    //calculate total tiles
    int totalTiles = cols * rows;

    // create the window
    unsigned width = cols * 32;
    unsigned height = (rows * 32) + 100;
    sf::RenderWindow welcomeWindow(sf::VideoMode({width, height}), "Welcome Window", sf::Style::Close);

    //Create arial font
    sf::Font arial;
    if (!arial.openFromFile("font.ttf")){
        cout << "Error loading font" << endl;
        return -1;
    }

    //title
    sf::Text title(arial);
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Underlined);
    title.setString("Welcome to Minesweeper!");

    setText(title, width/2, (height/2)-150);

    //subtitle
    sf::Text subtitle(arial);
    subtitle.setCharacterSize(20);
    subtitle.setFillColor(sf::Color::White);
    subtitle.setString("Enter your name: ");

    setText(subtitle, width/2, (height/2)-75);


    //name
    sf::Text name(arial);
    name.setCharacterSize(18);
    name.setFillColor(sf::Color::White);

    string playerName;

    // run the welcome screen as long as it's open
    while (welcomeWindow.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = welcomeWindow.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                welcomeWindow.close();
                return 0;
            }



            //handles if enter is pressed
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && !(playerName.empty())){
                welcomeWindow.close();
                break;
            }

            // handles text entered events
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                //backspace
                if (textEntered->unicode == 8) { // 8 is ASCII for backspace
                    if (!playerName.empty()) {
                        playerName.pop_back();
                    }
                }

                //if more than 10 characters
                else if (playerName.size() > 9) {
                    continue;
                }

                //letter
                else if (textEntered->unicode < 128) {
                    char enteredChar = static_cast<char>(textEntered->unicode);
                    if (isalpha(enteredChar)) {
                        playerName += enteredChar;
                    }
                }
            }
        }
        //capitalize name
        welcomeWindow.clear(sf::Color::Blue);
        string uppercaseName = toUppercase(playerName);
        uppercaseName.push_back(static_cast<char>('|'));
        name.setString(uppercaseName);

        //title and subtitle
        welcomeWindow.draw(title);
        welcomeWindow.draw(subtitle);
        setText(name, width/2, (height/2)-45);

        welcomeWindow.draw(name);
        // end the current frame
        welcomeWindow.display();
    }

    //game window

    float gameRows = static_cast<float>(rows);
    float gameCols = static_cast<float>(cols);

    sf::RenderWindow gameWindow(sf::VideoMode({width, height}), "Game Window", sf::Style::Close);


    // Create and set position of menu sprites

    // happy face/win/lose sprite (all same position but changes based on condition)
    sf::Texture happyFace("../../files/images/face_happy.png");
    sf::Texture winFace("../../files/images/face_win.png");
    sf::Texture loseFace("../../files/images/face_lose.png");

    sf::Sprite currentFace(happyFace);
    currentFace.setPosition({((gameCols/2.0f) * 32) - 32, 32 * (gameRows + 0.5f)});

    sf::Texture debug("../../files/images/debug.png");
    sf::Sprite debugSprite(debug);
    debugSprite.setPosition({(gameCols * 32.0f) - 304, 32 * (gameRows + 0.5f)});

    // pause/play button
    sf::Texture play("../../files/images/play.png");
    sf::Texture pause("../../files/images/pause.png");
    sf::Sprite controlButton(pause);
    controlButton.setPosition({(gameCols * 32.0f) - 240, 32 * (gameRows + 0.5f)});

    sf::Texture leaderboard("../../files/images/leaderboard.png");
    sf::Sprite leaderboardSprite(leaderboard);
    leaderboardSprite.setPosition({(gameCols * 32.0f) - 176, 32 * (gameRows + 0.5f)});

    //obtain each digit
    vector<sf::Sprite> digits;
    sf::Texture allDigits("../../files/images/digits.png");

    int digitHeight = allDigits.getSize().y;
    int digitWidth = 21;
    int counter = mines;


    for (int i = 0; i < 11; i++) {
        sf::Sprite digitSprites(allDigits);

        sf::Vector2i position(i * digitWidth, 0);
        sf::Vector2i size(digitWidth, digitHeight);
        sf::IntRect rect(sf::IntRect(position, size));

        sf::Sprite digitSprite(allDigits, rect);
        digits.push_back(digitSprite);
    }


    // load all numbers
    vector<sf::Texture> nums;
    for (int i = 1; i <= 8; i++) {
        nums.emplace_back("../../files/images/number_" + to_string(i) + ".png");
    }

    sf::Texture flag("../../files/images/flag.png");
    sf::Texture mine("../../files/images/mine.png");
    sf::Texture hidden("../../files/images/tile_hidden.png");
    sf::Texture shown("../../files/images/tile_revealed.png");

    vector<Tile> tiles;

    // add set number of mines
    for (int i = 0; i < mines; i++) {
        Tile currTile(nums, &hidden, &shown, &flag, &mine, true);
        tiles.push_back(currTile);
    }

    //add remaining tiles
    for (int i = 0; i < totalTiles - mines; i++) {
        Tile currTile(nums, &hidden, &shown, &flag, &mine, false);
        tiles.push_back(currTile);
    }

    //randomize mines and tiles
    randomize(tiles, cols, rows);


   // run game screen as long as it's open
    bool gameOver = false;
    bool victory = false;

    auto startTime = chrono::high_resolution_clock::now(); // begins timer
    auto totalPauseTime = std::chrono::seconds(0); // holds total paused time
    std::chrono::high_resolution_clock::time_point currentTime; // actual time elapsed (find by subtracting paused)
    std::chrono::high_resolution_clock::time_point pauseStart;

    bool isPaused = false;
    bool isLeaderOpen = false;
    while (gameWindow.isOpen()) {
         // check all the window's events that were triggered since the last iteration of the loop
         while (const std::optional event = gameWindow.pollEvent())
         {
             if (event->is<sf::Event::Closed>()) {
                 gameWindow.close();
                 return 0;
             }

             if (!gameOver) {
                 if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                     // checks for UI buttons
                     if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                         // get click
                         auto click = static_cast<sf::Vector2f>(mouseButtonPressed->position);

                         // always allow face, control, and leaderboard buttons
                         if (currentFace.getGlobalBounds().contains(click)){
                             randomize(tiles, cols, rows);
                             for (int i = 0; i < tiles.size(); i++) {
                                 tiles[i].hide();
                                 tiles[i].setVal();
                             }

                         } else if (controlButton.getGlobalBounds().contains(click)) {
                             if (isPaused == false) {
                                 isPaused = true;
                                 pauseStart = chrono::high_resolution_clock::now();
                                 cout << "Paused" << endl;
                             } else {
                                 auto resumeTime = chrono::high_resolution_clock::now();
                                 auto pauseDuration = chrono::duration_cast<chrono::seconds>(resumeTime - pauseStart);
                                 totalPauseTime += pauseDuration;
                                 isPaused = false;
                                 cout << "Resumed" << endl;
                             }
                             // converts all to empty tiles when leaderboard is open
                         } else if (leaderboardSprite.getGlobalBounds().contains(click)) {
                             for (Tile &currTile : tiles) {
                                 currTile.leaderboardState();
                             }
                         }
                         if (!isPaused && !isLeaderOpen) {
                             // debug handler
                             if (debugSprite.getGlobalBounds().contains(click)){
                                 for (int i = 0; i < tiles.size(); i++) {
                                     if (tiles[i].checkMine()) {
                                         tiles[i].reveal();
                                     }
                                 }
                             } else {
                                 // handles tiles
                                 for (int i = 0; i < tiles.size(); i++) {
                                     // finds tile that is clicked
                                     if (tiles[i].contains(click)) {
                                         // if tile is a mine, then reveal all mines
                                         if (tiles[i].checkMine()) {
                                             for (Tile& tile : tiles) {
                                                 if (tile.checkMine()) {
                                                     tile.reveal();
                                                 }
                                             }
                                             gameOver = true;
                                             break;
                                         }
                                         // if no mines nearby (empty tile), flood reveal
                                         // flood reveal checks all tiles if empty, if val > 0, then stop flood
                                         else {
                                             tiles[i].floodReveal();
                                         }
                                         break;
                                     }
                                 }
                             }
                         }
                     }
                     if (mouseButtonPressed->button == sf::Mouse::Button::Right) {
                         for (int i = 0; i < tiles.size(); i++) {
                             // sets/removes flag on tile right click
                             if (tiles[i].contains(sf::Vector2f(mouseButtonPressed->position))) {
                                 if (tiles[i].hasFlag()) {
                                     tiles[i].removeFlag();
                                     counter += 1;
                                 } else {
                                     tiles[i].setFlag();
                                     counter -= 1;
                                 }
                             }
                         }
                     }
                 }
             }
         }

        //win check (after a setFlag)
        for (Tile &tile : tiles) {
            if (tile.checkMine() && tile.hasFlag()) {
                victory = true;
        } else if (tile.checkMine() && !tile.hasFlag()) {
            victory = false;
            break;
            }
        }
        if (victory) {
            gameOver = true;
        }

        gameWindow.clear(sf::Color::White);

         //tile sprites
        for (int i = 0; i < tiles.size(); i++) {
            tiles[i].draw(gameWindow);
        }

        //draws UI bar
        gameWindow.draw(currentFace);
        gameWindow.draw(debugSprite);
        gameWindow.draw(controlButton);
        gameWindow.draw(leaderboardSprite);

        // counter
        sf::Vector2f digitPOS(33, 32 * (rows+0.5f)+16.0f);
        string counterStr = to_string(counter);
        int start = 0;
        if (counter < 0) {
            digitPOS.x -= 21;
            digits[10].setPosition(digitPOS);
            gameWindow.draw(digits[10]);
            digitPOS.x += 21;
            start++;
        }

        for (int i = start; i < counterStr.size(); i++) {
            int currDigit = counterStr[i] - '0';
            digits[currDigit].setPosition(digitPOS);
            gameWindow.draw(digits[currDigit]);
            digitPOS.x += 21;
        }
        // pause/unpause
        if (isPaused) {
            currentTime = pauseStart;  // frozen while paused
            controlButton.setTexture(play);

        } else {
            currentTime = chrono::high_resolution_clock::now();  // updated unpaused
            controlButton.setTexture(pause);
        }

        // takes elapsed time and converts to minutes and seconds
        auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime - totalPauseTime);
        int min = elapsedTime.count() / 60;
        int sec = elapsedTime.count() % 60;

        sf::Vector2f minPos(cols * 32 - 97, 32 * (rows +0.5f) + 16.0f);
        sf::Vector2f secPos(cols * 32 - 54, 32 * (rows + 0.5f) + 16.0f);

        // minutes
        minPos.x += 21;
        sf::Sprite minDigit2 = digits[min%10];
        minDigit2.setPosition(minPos);
        min /= 10;
        minPos.x -= 21;
        sf::Sprite minDigit1 = digits[min];
        minDigit1.setPosition(minPos);

        // seconds
        secPos.x += 21;
        sf::Sprite secDigit2 = digits[sec%10];
        secDigit2.setPosition(secPos);
        sec /= 10;
        secPos.x -= 21;
        sf::Sprite secDigit1 = digits[sec];
        secDigit1.setPosition(secPos);

        gameWindow.draw(minDigit1);
        gameWindow.draw(minDigit2);
        gameWindow.draw(secDigit1);
        gameWindow.draw(secDigit2);

        // checks if gameOver and dictates victory faces
        if (gameOver) {
            if (victory) {
                currentFace.setTexture(winFace);
            } else {
                currentFace.setTexture(loseFace);
            }
        }

        // end the current frame
        gameWindow.display();
     }
}
