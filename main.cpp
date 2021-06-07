#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>
#include <SFML/Graphics.hpp>

struct Connect4Game {
    static const int WIDTH{ 7 };
    static const int HEIGHT{ 6 };

    int board[HEIGHT][WIDTH]; // 0 is empty, 1 is blue, 2 is red
    int lastAction;
    bool redTurn; // is it red's turn (false for blue)


    Connect4Game(int playerOneColor) : lastAction{ -1 } {
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
                board[i][j] = 0;

        if (playerOneColor == BLUE) redTurn = false;
        else redTurn = true;
    }

    // location is between 0 and WIDTH
    // returns whether it successfully placed a tile
    bool placeTile(int location) {
        for (int y = HEIGHT - 1; y >= 0; --y)
            if (board[y][location] == EMPTY) {
                if (redTurn) board[y][location] = RED;
                else board[y][location] = BLUE;
                redTurn = !redTurn; // switch it to the other player's turn
                lastAction = location; // and update the last action
                return true;
            }

        // if that column was full, then return false
        return false;
    }

    // This can probably be written better, but whatever
    int getWinner() {
        int numberOfFullTiles{ 0 };

        // Check for horizontal win
        for (int row = 0; row < HEIGHT; ++row) {
            for (int col = 0; col < WIDTH - 3; ++col) {

                if (board[row][col] != EMPTY) {
                    int checkColor = board[row][col];
                    for (int i = 1; i < 4; ++i) {
                        // if the color isn't the same as the initial check, then goto the next check
                        if (board[row][col + i] != checkColor) goto NEXT_HORIZ_TILE;
                    }
                    return checkColor; // if it checked all four and they were the same, return the winner
                }

            NEXT_HORIZ_TILE:;
            }

        }

        //Check for vertical win
        for (int col = 0; col < WIDTH; ++col) {
            for (int row = 0; row < HEIGHT - 3; ++row) {

                if (board[row][col] != EMPTY) {
                    int checkColor = board[row][col];
                    for (int i = 1; i < 4; ++i) {
                        // if the color isn't the same as the initial check, then goto the next check
                        if (board[row + i][col] != checkColor) goto NEXT_VERT_TILE;
                    }
                    return checkColor; // if it checked all four and they were the same, return the winner
                }

            NEXT_VERT_TILE:;

            }
        }

        //Check for diagonal right win
        for (int col = 0; col < WIDTH - 3; ++col) {
            for (int row = 0; row < HEIGHT - 3; ++row) {

                if (board[row][col] != EMPTY) {
                    int checkColor = board[row][col];
                    for (int i = 1; i < 4; ++i) {
                        // if the color isn't the same as the initial check, then goto the next check
                        if (board[row + i][col + i] != checkColor) goto NEXT_DIAG_TILE;
                    }
                    return checkColor; // if it checked all four and they were the same, return the winner
                }

            NEXT_DIAG_TILE:;

            }
        }

        //Check for diagonal left win //////////////////////////////////////////////////////bad
        for (int col = 0; col < WIDTH - 3; ++col) {
            for (int row = 3; row < HEIGHT; ++row) {

                if (board[row][col] != EMPTY) {
                    int checkColor = board[row][col];
                    for (int i = 1; i < 4; ++i) {
                        // if the color isn't the same as the initial check, then goto the next check
                        if (board[row - i][col + i] != checkColor) goto NEXT_DIAG_LEFT_TILE;
                    }
                    return checkColor; // if it checked all four and they were the same, return the winner
                }

            NEXT_DIAG_LEFT_TILE:;

            }
        }


        // Check for tie
        for (int row = 0; row < HEIGHT; ++row)
            for (int col = 0; col < WIDTH; ++col)
                if (board[row][col] != EMPTY)
                    ++numberOfFullTiles;

        if (numberOfFullTiles == WIDTH * HEIGHT) return TIE;
        return STILL_PLAYING;
    }

    enum Tiles {
        EMPTY,
        RED,
        BLUE
    };

    enum WinningStates {
        STILL_PLAYING, // game still going
        RED_WIN,       // red wins
        BLUE_WIN,      // blue wins
        TIE            // game over but a tie
    };
};


struct Node {
    bool visited{ false }; // whether it has been played out
    bool expanded{ false };

    int visits{ 0 };
    float value{ 0 }; // +1 for win, +0.5 for tie, +0 for loss
    int action;      // the action that got this state to what it is from the parent state

    Connect4Game state;
    Node* parent;
    std::vector<Node> children;

    Node(const Connect4Game& game, Node* par, int act) :
        state{ game }, parent{ par }, action{ act } {
        children.reserve(Connect4Game::WIDTH);
    }




};




void backpropagateAndUpdate(Node* node, float value) {

    Node* current = node;

    // update the current node's visits and value
    while (true) {
        current->visits++;
        current->value += value;

        if (!current->parent) return;
        current = current->parent;
    }
}

// fully expand a node so that its children are full
void expand(Node* node, int aiColor) {
    // if it's already fully expanded, then return
    if (node->children.size() == Connect4Game::WIDTH) return;

    node->expanded = true;

    for (int action = 0; action < Connect4Game::WIDTH; ++action) {
        Connect4Game newState{ node->state };
        if (!newState.placeTile(action)) continue; // if it can't place the tile in that spot, then continue

        Node child{ newState, node, action };
        node->children.push_back(child);


    }



}

//https://en.wikipedia.org/wiki/Monte_Carlo_tree_search
float getUTCscore(Node& node) {


    float w = node.value;
    float n = (float)node.visits;
    float N = (float)node.parent->visits;
    float c = (float)sqrt(2);

    if (n == 0 || N == 0) return log(N);


    return (w / n) + c * sqrt((log(N) / n));

}
// calculates the utc score of every child of the node passed in
// and returns the highest one
Node* selectChild(Node& parent) {
    if (parent.children.size() == 0) {
        std::cout << "ERROR: parent being selected from has no children\n";
        return nullptr;
    }


    float highestScore = -1000;
    Node* selection = nullptr;

    for (size_t i = 0; i < parent.children.size(); ++i) {

        float score = getUTCscore(parent.children[i]);

        if (score > highestScore) {
            highestScore = score;
            selection = &parent.children[i];
        }
    }

    return selection;



}

float rollout(const Node& node, int aiColor) {
    Node simulate = node;

    while (true) {
        int winner = simulate.state.getWinner();

        if (winner != Connect4Game::STILL_PLAYING) { // if it isn't still playing
            //then return the winner
            if (winner == aiColor) return 1.f;
            else if (winner == Connect4Game::TIE) return 0.f;
            else return -1.f;
        }
        // if it is still playing
        int randMove = rand() % Connect4Game::WIDTH;
        simulate.state.placeTile(randMove);

    }
}


// https://towardsdatascience.com/game-ais-with-minimax-and-monte-carlo-tree-search-af2a177361b0:
//1. Start at the root node and use the UCT formula to calculate the score for every child node
//2. Pick the child node for which you’ve computed the highest UCT score
//3. Check if the child has already been visited
//4. If not, do a rollout
//5. If yes, determine the potential next states from there, use the UCT formula to decide which child node to pick and do a rollout
//6. Propagate the result back through the tree until you reach the root node
//7. Go back to step 1
int MonteCarloTreeSearch(const Connect4Game& game, int aiColor, int ms = 100) {


    int color = aiColor;
    int opponentColor = Connect4Game::RED;
    if (color == opponentColor) opponentColor = Connect4Game::BLUE;

    int iterations = 0;

    Node root{ game, nullptr, game.lastAction };
    expand(&root, color);

    // start of by doing rollout on every child of the root
    for (auto& i : root.children) {
        float score = rollout(i, aiColor);
        // and backpropagate
        backpropagateAndUpdate(&i, score);
        i.visited = true;
    }


    auto before = std::chrono::system_clock::now();
    while (before + std::chrono::milliseconds(ms) > std::chrono::system_clock::now()) {
        ++iterations;



        // while the selection has already been expanded
        // if that child has already been expanded then make the selection one of its children
        // and so on until you reach an unexpanded node
        Node* selection = &root;
        while (selection->expanded) {
            selection = selectChild(*selection);
        }

        expand(selection, color);
        for (auto& i : selection->children) {
            float score = rollout(i, aiColor);
            // and backpropagate
            backpropagateAndUpdate(&i, score);
            i.visited = true;
        }


    }


    float highestScore = -100000;
    int bestAction = -1;
    int bestIndex = -1;

    for (size_t i = 0; i < root.children.size(); ++i) {
        float score = root.children[i].value / root.children[i].visits;

        int winner = root.children[i].state.getWinner();

        // if it wins from this move, then return it
        if (winner == aiColor) {
            bestAction = root.children[i].action;
            bestIndex = i;
            break;
        }
        // if it loses from this move, then don't return it
        if (aiColor == Connect4Game::RED && winner == Connect4Game::BLUE_WIN)
            continue;
        if (aiColor == Connect4Game::BLUE && winner == Connect4Game::RED_WIN)
            continue;

        bool childIsLosing{ false };
        for (int c = 0; c < root.children[i].children.size(); ++c) {
            int childWinner = root.children[i].children[c].state.getWinner();
            if (aiColor == Connect4Game::RED && childWinner == Connect4Game::BLUE_WIN)
                childIsLosing = true;
            if (aiColor == Connect4Game::BLUE && childWinner == Connect4Game::RED_WIN)
                childIsLosing = true;
        }
        if (childIsLosing) continue;


        if (score > highestScore) {
            highestScore = score;
            bestIndex = i;
            bestAction = root.children[i].action;
        }
    }

    std::cout << "Estimated propability of AI winning: " <<
        root.children[bestIndex].value / root.children[bestIndex].visits * 100 << "%\n";
    std::cout << "Iterations: " << iterations << '\n';


    return bestAction;
}




void renderConnect4GameInConsole(Connect4Game& game) {
    std::cout << "\x1B[2J\x1B[H"; // clear screen better

    for (int row = 0; row < Connect4Game::HEIGHT; ++row) {
        for (int col = 0; col < Connect4Game::WIDTH; ++col) {
            char c{ '.' };
            if (game.board[row][col] == Connect4Game::RED)   c = '#';
            if (game.board[row][col] == Connect4Game::BLUE)  c = 'O';
            if (game.board[row][col] == Connect4Game::EMPTY) c = '.';

            std::cout << c << " ";

        }
        std::cout << '\n';
    }

    for (int col = 0; col < Connect4Game::WIDTH; ++col)
        std::cout << col + 1 << " ";

}

void playConnect4GameInConsole() {
RESTART:

    int playerColor = Connect4Game::RED;
    int opponentColor = Connect4Game::BLUE;
    Connect4Game game{ playerColor };

    char playerCharacter{ 'O' };
    if (playerColor == Connect4Game::RED) playerCharacter = '#';

    while (true) {


        renderConnect4GameInConsole(game);


        if (game.getWinner() != Connect4Game::STILL_PLAYING) {
            renderConnect4GameInConsole(game);
            switch (game.getWinner()) {
            case Connect4Game::RED_WIN:
                std::cout << "RED WINS" << '\n';
                break;
            case Connect4Game::BLUE_WIN:
                std::cout << "BLUE WINS" << '\n';
                break;
            case Connect4Game::TIE:
                std::cout << "TIE" << '\n';
                break;
            default:
                break;
            }
            std::cout << "Would you like to play again? (1 for yes, 0 for no): ";
            bool again{ false };
            std::cin >> again;
            if (again) {
                goto RESTART; // i know that gotos are bad and whatnot, but whatever
            }
            else
                return;
        }

        std::cout << "\nWhich column will you place (You are: " << playerCharacter << "): ";
        int row{ -1 };
        std::cin >> row;


        while (!game.placeTile(row - 1)) {
            std::cout << "Try again: ";
            std::cin >> row;
        }

        game.placeTile(MonteCarloTreeSearch(game, opponentColor, 100));


    }

}



void renderConnect4Game(Connect4Game& game, sf::Vector2f location, float tileSize, sf::RenderWindow& window) {
    sf::CircleShape circle(tileSize/2);

    for (int y = 0; y < Connect4Game::HEIGHT; ++y)
        for (int x = 0; x < Connect4Game::WIDTH; ++x)
            if (game.board[y][x] != Connect4Game::EMPTY) {
                circle.setPosition(x * tileSize + location.x, y * tileSize + location.y);
                if (game.board[y][x] == Connect4Game::RED) circle.setFillColor(sf::Color::Red);
                if (game.board[y][x] == Connect4Game::BLUE) circle.setFillColor(sf::Color::Blue);
                window.draw(circle);
            }


    sf::RectangleShape verticalLine{ {2.f, tileSize * Connect4Game::HEIGHT} };
    sf::RectangleShape horizontalLine{ { tileSize * Connect4Game::WIDTH, 2.f } };


    for (int y = 0; y < Connect4Game::HEIGHT + 1; ++y) {
        horizontalLine.setPosition(location.x, location.y + y * tileSize);
        window.draw(horizontalLine);
    }

    for (int x = 0; x < Connect4Game::WIDTH + 1; ++x) {
        verticalLine.setPosition(location.x + x * tileSize, location.y);
        window.draw(verticalLine);
    }
}


int main() {

    //playConnect4GameInConsole();

    float tileSize{ 50.f };
    sf::Vector2f boardPos{ 100, 100 };


    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");

    int playerColor = Connect4Game::RED;
    int opponentColor = Connect4Game::BLUE;
    Connect4Game game{ playerColor };


    while (window.isOpen()) {
        sf::Vector2f mouse{ (float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y };



        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int column = static_cast<int>((mouse.x - boardPos.x) / tileSize);
                    if (column >= 0 && column < Connect4Game::WIDTH)
                        if(game.placeTile((int)column))
                            game.placeTile(MonteCarloTreeSearch(game, opponentColor, 100));



                    switch (game.getWinner()) {
                    case Connect4Game::RED_WIN:
                        std::cout << "RED WINS" << '\n';
                        break;
                    case Connect4Game::BLUE_WIN:
                        std::cout << "BLUE WINS" << '\n';
                        break;
                    case Connect4Game::TIE:
                        std::cout << "TIE" << '\n';
                        break;
                    default:
                        break;
                    }
                }

            }
        }



        window.clear();



        renderConnect4Game(game, boardPos, tileSize, window);


        window.display();
    }




    return 0;
}
