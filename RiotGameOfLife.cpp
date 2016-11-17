#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <sstream>
#include <limits>
using namespace std;

typedef struct {
    int x;
    int y;
} Cell;

struct cell_hasher {
    size_t operator()(const Cell& c) const
    {
        // Taken from http://stackoverflow.com/questions/2634690/good-hash-function-for-a-2d-index
        int hash = 17;
        hash = ((hash + c.x) << 5) - (hash + c.x);
        hash = ((hash + c.y) << 5) - (hash + c.y);
        return hash;
    }
};

void nextFrame(unordered_set<Cell, cell_hasher>& aliveCells, unordered_set<Cell, cell_hasher>& cellsToCheck);
void getNeighbors(Cell cell, vector<Cell>& neighbors);

inline bool operator == (Cell const& lhs, Cell const& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

ostream &operator<<(ostream &output, const Cell &c) {
    output << "(" << c.x << "," << c.y << ")";
    return output;
}

int getInteger(const string& prompt) {
    string promptCopy = prompt;
    int value;
    while (true) {
        cout << promptCopy;
        string line;
        getline(cin, line);
        istringstream stream(line);
        stream >> value >> std::ws;
        if (!stream.fail() && stream.eof()) {
            break;
        }
        cout << "Sorry, you didn't enter a valid number. Please try again. " << endl;
    }
    return value;
}

//Helper functions to convert user input into cells:
int extractCoordinate(string cell, bool isX) {
    int startPos = isX ? cell.find("("): cell.find(",");
    int endPos = isX ? cell.find(",") : cell.find(")");
    if (startPos == -1 || endPos == -1) {
        throw -1;
    }
    startPos += 1;
    int coordinate;
    try {
        coordinate = stoi((cell.substr(startPos, endPos - startPos)), nullptr, 10);
    } catch (const out_of_range& oor) {
        (void)oor;
        throw -1;
    }
    return coordinate;
}

Cell strToCell(string cell) {
    string errMsg = "Sorry, one or more of the coordinates you entered was incorrectly formatted. Please try again.";
    Cell newCell;
    //A Cell is inputted in the format: (x,y)
    try {
        newCell.x = extractCoordinate(cell, true);
        newCell.y = extractCoordinate(cell, false);
    } catch (int e) {
        (void)e;
        throw errMsg;
    }
    return newCell;
}

void fastForwardFrames(int numFrames, unordered_set<Cell, cell_hasher>& aliveCells, unordered_set<Cell, cell_hasher>& cellsToCheck) {
    for (int i = 0; i < numFrames; i++) {
        nextFrame(aliveCells, cellsToCheck);
    }
}

/*
If an "alive" Cell had less than 2 or more than 3 alive neighbors (in any of the 8 surrounding Cells), it becomes dead.
If a "dead" Cell had *exactly* 3 alive neighbors, it becomes alive.
*/
void nextFrame(unordered_set<Cell, cell_hasher>& aliveCells, unordered_set<Cell, cell_hasher>& cellsToCheck) {
    unordered_set<Cell, cell_hasher> newAliveCells = aliveCells;
    unordered_set<Cell, cell_hasher> newCellsToCheck;
    //If current cell changed, add its neighbors to cells to check next time
    for (auto itr = cellsToCheck.begin(); itr != cellsToCheck.end(); ++itr) {
        Cell cell = *itr;
        vector<Cell> neighbors;
        getNeighbors(cell, neighbors);
        int finalCount = 0;
        bool dead = false;
        for (size_t i = 0; i < neighbors.size(); i++) {
            if (aliveCells.count(neighbors[i]) > 0) {
                finalCount++;
            }
            if (finalCount > 3) {
                dead = true;
                break;
            }
        }
        //If the cell becomes dead
        if (finalCount < 2 || dead) {
            if (newAliveCells.erase(cell) != 0) {
                //If it was alive before and is now dead, add its neighbors to new cells to check
                for (size_t i = 0; i < neighbors.size(); i++) {
                    newCellsToCheck.insert(neighbors[i]);
                }
            }
        //If the cell becomes alive
        } else if (finalCount == 3) {
            if (newAliveCells.insert(cell).second) {
                //If it was dead before and becomes alive, add its neighbors to new cells to check
                for (size_t i = 0; i < neighbors.size(); i++) {
                    newCellsToCheck.insert(neighbors[i]);
                }
            }
        }
    }
    aliveCells = newAliveCells;
    cellsToCheck = newCellsToCheck;
}

//Populates neighbors with neighbors of a given cell
void getNeighbors(Cell cell, vector<Cell>& neighbors) {
    int max_dc = cell.y == numeric_limits<int>::max() ? 0 : 1;
    int min_dc = cell.y == numeric_limits<int>::min() ? 0 : -1;
    int max_dr = cell.x == numeric_limits<int>::max() ? 0 : 1;
    int min_dr = cell.x == numeric_limits<int>::min() ? 0 : -1;

    for (int dr = min_dr; dr <= max_dr; dr++) {
        for (int dc = min_dc; dc <= max_dc; dc++) {
            if (dc == 0 && dr == 0) continue;
            Cell neighbor;
            neighbor.x = cell.x + dr;
            neighbor.y = cell.y + dc;
            neighbors.push_back(neighbor);
        }
    }
}

int main() {
    cout << "Please enter a list of alive (x,y) integer coordinates, each separated by a line break. Enter an empty line when finished." << endl;
    string line;
    vector<string> aliveCoordinates;
    while (getline(cin, line)) {
        if (line.length() == 0) {
            break;
        }
        aliveCoordinates.push_back(line);
    }

    unordered_set<Cell, cell_hasher> aliveCells;
    for (size_t i = 0; i < aliveCoordinates.size(); i++) {
        //Parse input into cells
        try {
            aliveCells.insert(strToCell(aliveCoordinates[i]));
        } catch (string e) {
            cerr << e << endl;
            exit(0);
        }
    }

    cout << "Here are the beginning alive cells. The Game of Life will now begin!:" << endl;
    unordered_set<Cell, cell_hasher> cellsToCheck; //Will keep track of cells that should be checked due to a recently changed neighbor
    for (auto itr = aliveCells.begin(); itr != aliveCells.end(); ++itr) {
        cellsToCheck.insert(*itr);
        vector<Cell> neighbors;
        getNeighbors(*itr, neighbors);
        for (size_t i = 0; i < neighbors.size(); i++) {
            cellsToCheck.insert(neighbors[i]);
        }
        cout << *itr << endl;
    }
    cout << endl;

    while (true) {
        cout << "Would you like to go to the next generation (Enter n) or skip generations (Enter s)? (Enter anything else to quit) " << endl;
        string input;
        getline(cin, input);
        if (input == "n") {
            nextFrame(aliveCells, cellsToCheck);
        } else if (input == "s") {
            int numFrames = getInteger("Enter the number of generations you would like to advance past: ");
            fastForwardFrames(numFrames, aliveCells, cellsToCheck);
        } else {
            cout << "See ya!" <<endl;
            break;
        }

        if (aliveCells.empty()) {
            cout << "There are no alive cells!" << endl;
        } else {
            cout << "Here are the alive cells:" << endl;
        }
        for (auto itr = aliveCells.begin(); itr != aliveCells.end(); ++itr) {
            cout << *itr << endl;
        }
        cout << endl;
    }
    return 0;
}
