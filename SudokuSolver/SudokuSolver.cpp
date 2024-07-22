// SudokuSolver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<chrono>
#include<vector>
#include "ThreadPool.h"
#include<mutex>

std::mutex mut;
bool solutionFnd = 0;

const int threadPoolSize = 100;
ThreadPool threadPool(threadPoolSize);
std::vector<std::vector<short int>> _sudokuBoard(9, std::vector<short int>(9, 0));

void printSudokuBoard() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++)
            std::cout << " ---";
        std::cout << "\n| ";
        for (int j = 0; j < 9; j++)
            std::cout << _sudokuBoard[i][j] << " | ";
        std::cout << "\n";
    }
    for (int j = 0; j < 9; j++)
        std::cout << " ---";
}

bool checkIsValidBoard(std::vector<std::vector<short int>> sudokuBoard) {
    //Check for Rows and Column
    for (int i = 0; i < 9; i++) {
        std::vector<bool> fndRow(10, 0);
        std::vector<bool> fndCol(10, 0);
        for (int j = 0; j < 9; j++) {
            if (sudokuBoard[i][j] != 0) {
                if (fndRow[sudokuBoard[i][j]]) return false;
                fndRow[sudokuBoard[i][j]] = 1;
            }
            if (sudokuBoard[j][i] != 0) {
                if (fndCol[sudokuBoard[j][i]]) return false;
                fndCol[sudokuBoard[j][i]] = 1;
            }
        }
    }

    //Check for each 3X3 block
    for (int r = 0; r < 7; r += 3) {
        for (int c = 0; c < 7; c += 3) {
            std::vector<bool> fnd(15, 0);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (sudokuBoard[r + i][c + j] != 0) {
                        if (fnd[sudokuBoard[r + i][c + j]]) return false;
                        fnd[sudokuBoard[r + i][c + j]] = 1;
                    }
                }
            }
        }
    }

    return true;
}

void solveSudoku(std::vector<std::vector<short int>> sudokuBoard, int x, int y) {
    if (solutionFnd) return;
    else if (x > 8) {   
        std::unique_lock<std::mutex> lock(mut);
        if (solutionFnd) {
            lock.unlock();
            return;
        }
        _sudokuBoard = sudokuBoard;
        threadPool.abortThreadPool();
        solutionFnd = 1;
        lock.unlock();
    }
    else if (sudokuBoard[x][y] != 0){
        std::vector<std::vector<short int>> board = sudokuBoard;
        threadPool.enqueueTask([board, x, y] {
            solveSudoku(board, x + ((y + 1) / 9), (y + 1) % 9);
            });
    }
    else {
        for (short int i = 1; i <= 9; i++) {
            sudokuBoard[x][y] = i;
            if (!checkIsValidBoard(sudokuBoard)) continue;
            std::vector<std::vector<short int>> board = sudokuBoard;
            threadPool.enqueueTask([board, x, y] {
                solveSudoku(board, x + ((y + 1) / 9), (y + 1) % 9);
                });
            sudokuBoard[x][y] = 0;
        }
    }
}

bool solve() {
    threadPool.enqueueTask([] {
        solveSudoku(_sudokuBoard, 0, 0);
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    while (!threadPool.isAbort());
    return solutionFnd;
}

int main()
{
    std::cout << "Enter a 9x9 Sudoku Board: \n";
    for (short i = 0; i < 9; i++)
        for (short j = 0; j < 9; j++)
            std::cin >> _sudokuBoard[i][j];

    if (solve()) {
        std::cout << "\nSolution: \n";
        printSudokuBoard();
    }
    else std::cout << "Enter a valid sudoku board :(...\n";

    return 0;
}