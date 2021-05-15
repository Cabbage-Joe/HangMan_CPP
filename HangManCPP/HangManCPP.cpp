// HangManCPP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <map>
#include <stdlib.h>
#include <chrono>
#include <thread>


using namespace std;

namespace HangMan {
    /* Overloading * operator */
    string operator * (string a, unsigned int b) {
        string output = "";
        while (b--) {
            output += a;
        }
        return output;
    }

    // Constants
    #define MAX_STRIKES       7         /* Max number of incorrect guesses*/
    #define MAX_SPACE_AFTER_WORD_STATUS 28
    #define INCORRECT_GUESS_NUM_EASY    10  /* Max number of incorrect  */
                                            /* guesses for easy mode    */
    #define INCORRECT_GUESS_NUM_MEDIUM  8   /* Max number of incorrect  */
                                            /* guesses for medium mode  */
    #define INCORRECT_GUESS_NUM_HARD    5   /* Max number of incorrect  */
                                            /* guesses for hard mode    */
    #define INCORRECT_GUESS_NUM_INSANE  2   /* Max number of incorrect  */
                                            /* guesses for insane mode  */
    #define NUM_HANGMAN_PARTS 7             /* The number of parts that */
                                            /* make up a full hang man  */
                                            /* graphic                  */
    class Letter {
    
    // Members
    public: 
        char cData;
        bool bIsGuessed;
        Letter* lNext;
    private:
        
    
    // Methods
    public:
        Letter(char cDataP, Letter* lNextP) { 
            this->cData = cDataP;
            this->lNext = lNextP;
        }
        ~Letter() {
            // Anything needed?
        }
        void Display() { // Used for debug
            cout << "cData = " << cData << " bIsGuessed = " << bIsGuessed << endl;
        }
    };

    class HangManGame {

        enum class Difficulty
        {
            Easy = 1,
            Medium = 2,
            Hard = 3,
            Insane = 4
        };

        class GameWord {

        private:
            Letter* _lHead;
            Letter* _lEnd;

        public:
            bool bWordHasBeenGuessed = false;
            void PrintListContents() { // For debugging
                Letter* lEle = _lHead;
                do {
                    lEle->Display();
                    lEle = lEle->lNext;
                } while (lEle != NULL);
                std::cout << endl;
            }

            string GetWord() {
                string strRet = "";

                Letter* lEle = _lHead;
                do {
                    strRet += lEle->cData;
                    lEle = lEle->lNext;
                } while (lEle != NULL);

                return strRet;
            }

            string GetWordStatus(){
                string strRet = "";

                Letter* lEle = _lHead;
                do {
                    if (lEle->bIsGuessed) {
                        strRet += lEle->cData;
                    }
                    else {
                        strRet += "X";
                    }
                    lEle = lEle->lNext;
                } while (lEle != NULL);

                return strRet;
            }

            GameWord(string strGameWord) {
                bool bFirstPass = true;
                for (auto cEle : strGameWord)
                {
                    if (bFirstPass) {
                        // Create the head
                        _lHead = new Letter(cEle, NULL);

                        // Point the end to the head
                        // because at this point the length is 1
                        _lEnd = _lHead;

                        bFirstPass = false;
                    }
                    else {
                        // Create new linked list entry
                        Letter* lNewEle = new Letter(cEle, NULL);

                        // Link the next pointer of the current end to new entry
                        _lEnd->lNext = lNewEle;

                        // Make the new entry the new end
                        _lEnd = lNewEle;
                    }
                }
            }
            ~GameWord() {
                // Not needed
            }

            bool Guess(char cGuess) {
                Letter* lEle = _lHead;
                bool bGuessIsContainedInWord = false;
                bWordHasBeenGuessed = true;
                do {
                    if (lEle->cData == cGuess) {
                        lEle->bIsGuessed = bGuessIsContainedInWord = true;
                    }
                    if (!lEle->bIsGuessed) {
                        bWordHasBeenGuessed = false;
                    }
                    lEle = lEle->lNext;
                } while (lEle != NULL);

                return bGuessIsContainedInWord;
            }
        private:
        };

    private:
        GameWord* _gmWord;
        int _iInvalidGuesses = 0;
        map<int, int> _mSpaceMap =      {{0, 4,}, // InvalidGuessAmount,# Of Blank Lines
                                        {1, 3,},
                                        {2, 2,},
                                        {3, 2,},
                                        {4, 2,},
                                        {5, 1,},
                                        {6, 0,},
                                        {7, 0,}};

        map<int,string> mWordsToBeGuessed = {   {0, "abruptly",},
                                                {1, "absurd",},
                                                {2, "puzzling",},
                                                {3, "espionage",},
                                                {4, "euouae",},
                                                {5, "fluffiness",},
                                                {6, "twelfths",},
                                                {7, "jukebox",},
                                                {8, "zombie",}, 
                                                {9, "knapsack",}, 
                                                {10, "jackpot",}, 
                                                {11, "joe",}, 
                                                {12, "fixable",}, 
                                                {13, "fishhook",}, 
                                                {14, "buzzword",}, 
                                                {15, "numbskull",}, 
                                                {16, "marquis",},};
    
    public:

        void StartGameLoop() {
            char iPlayAgainAnswer;

            do {
                string strGameWord = "";

                // Display main screen
                int iMaxIncorrectGuess;
                Difficulty dGameMode = DisplayIntroAndGetDifficulty();
                iMaxIncorrectGuess = GetIncorrectGuessAmount(dGameMode);
                std::cout << endl << "You will have " << iMaxIncorrectGuess << " incorrect guesses before you loose the game";
                std::cout << endl << "Good luck!";

                // Generate word to be guessed from the dictionary
                std::cout << endl << "Generating a random word for you to guess ...";
                srand((unsigned)time(NULL));
                int iWordDictionarySize = mWordsToBeGuessed.size();
                int iRandomNumber = (rand() % (iWordDictionarySize));
                strGameWord = mWordsToBeGuessed[iRandomNumber];
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Set up linked list
                _gmWord = new GameWord(strGameWord);

                // Start game
                int iGuessAttempts;
                int iCurrentIncorrectGuesses = 0;
                bool bWon = false;
                bool bLost = false;
                do {
                    char cGuess;
                    bLost = iCurrentIncorrectGuesses >= iMaxIncorrectGuess;

                    // If the game is not lost make a new guess
                    if (!bLost) {
                        bool bGuessIsCorrect;

                        cGuess = tolower(DisplayHangManStatusAndGetNextGuess(iCurrentIncorrectGuesses, iMaxIncorrectGuess, _gmWord));
                        bWon = _gmWord->bWordHasBeenGuessed;
                        bGuessIsCorrect = _gmWord->Guess(cGuess);
                        iCurrentIncorrectGuesses += (bGuessIsCorrect) ? 0 : 1;
                    }
                } while (bWon == false && bLost == false);

                if (bWon) {
                    // Display Congratulations screen
                    DisplayCongratulations();
                }
                else {
                    // Display defeat screen
                    DisplayDefeat();
                }

                // See if player wants to play again
                std::cout << endl << endl << "Would you like to play again? y/n";
                std::cin >> iPlayAgainAnswer;
                iPlayAgainAnswer = tolower(iPlayAgainAnswer);

            } while (iPlayAgainAnswer != 'n');   
        }

        Difficulty DisplayIntroAndGetDifficulty() {
            Difficulty dRet;
            
            system("CLS");
            std::cout << endl << "    __  __ ___     _   __ ______ __  ___ ___     _   __";
            std::cout << endl << "   / / / //   |   / | / // ____//  |/  //   |   / | / /";
            std::cout << endl << "  / /_/ // /| |  /  |/ // / __ / /|_/ // /| |  /  |/ / ";
            std::cout << endl << " / __  // ___ | / /|  // /_/ // /  / // ___ | / /|  /  ";
            std::cout << endl << "/_/ /_//_/  |_|/_/ |_/ \\____//_/  /_//_/  |_|/_/ |_/  ";
            std::cout << endl << "Select a difficulty to play:";
            std::cout << endl << "1 -> Easy";
            std::cout << endl << "2 -> Medium";
            std::cout << endl << "3 -> Hard";
            std::cout << endl << "4 -> Insane";
            std::cout << endl;

            int iDifficulty;
            cin >> iDifficulty; // TODO

            dRet = static_cast<Difficulty>(iDifficulty);

            return dRet;
        }

        int GetIncorrectGuessAmount(Difficulty dMode) {
            int iIncorrectGuessAmount = 0;

            switch (dMode)
            {
            case HangManGame::Difficulty::Easy:
            default:
                iIncorrectGuessAmount = INCORRECT_GUESS_NUM_EASY;
                break;
            case HangManGame::Difficulty::Medium:
                iIncorrectGuessAmount = INCORRECT_GUESS_NUM_MEDIUM;
                break;
            case HangManGame::Difficulty::Hard:
                iIncorrectGuessAmount = INCORRECT_GUESS_NUM_HARD;
                break;
            case HangManGame::Difficulty::Insane:
                iIncorrectGuessAmount = INCORRECT_GUESS_NUM_INSANE;
                break;
            }

            return iIncorrectGuessAmount;
        }

        char DisplayHangManStatusAndGetNextGuess(int iCurrentIncorrectGuesses,int iMaxIncorrectGuesses, GameWord* gmWord) {
            char cRet;
            string strWordStatus = gmWord->GetWordStatus();
            string strSpace = " ";
            int iWordStatusLength = strWordStatus.length();

            // Get progress of hangman
            double dInvalidGuessProgessRatio = ((double)iCurrentIncorrectGuesses / iMaxIncorrectGuesses);
            int iHangManPartsProgress = (int)(dInvalidGuessProgessRatio * NUM_HANGMAN_PARTS);

            // Display hang man status header
            system("CLS"); 
            std::cout << "\n=======================================================";
            std::cout << "\n=                   WORD: " + gmWord->GetWordStatus() + (strSpace * (MAX_SPACE_AFTER_WORD_STATUS - iWordStatusLength)) + "=";
            
            if (iHangManPartsProgress >= 1) {
                std::cout << "\n=                    (_)                              =";
                if (iHangManPartsProgress >= 2) {
                    std::cout << "\n=                    \\";
                    if (iHangManPartsProgress >= 3) {
                        std::cout << "|";
                        if (iHangManPartsProgress >= 4) {
                            std::cout << "/                              =";
                            if (iHangManPartsProgress >= 5) {
                                std::cout << "\n=                     |                               =";
                                if (iHangManPartsProgress >= 6) {
                                    std::cout << "\n=                    /";
                                    if (iHangManPartsProgress >= 7) {
                                        std::cout << " \\                              ="; // TODO - Delete? this case should will never be met
                                    }
                                }
                            }
                        }
                    }
                }
            } 
            string strBlankLine = "\n=                                                     =";
           
            // Pad the figure with new lines so no mater
            // what the figure display is always the same amount of lines
            std::cout << (strBlankLine * _mSpaceMap[iHangManPartsProgress]) << "\n";
            std::cout << endl << endl << "Please enter in a guess: ";
            cin >> cRet;

            return cRet;
        }    
        void DisplayCongratulations() {
            // Display congratulations screen 
            system("CLS");
            std::cout << endl << "   You have Won!";
            std::cout << endl << "                                  _ ";
            std::cout << endl << "                                 | |";
            std::cout << endl << "   ___ ___  _ __   __ _ _ __ __ _| |_ ___";
            std::cout << endl << "  / __/ _ \\| '_ \\ / _` | '__/ _` | __/ __|";
            std::cout << endl << " | (_| (_) | | | | (_| | | | (_| | |_\\__ \\ ";
            std::cout << endl << "  \\___\\___/|_| |_|\\__, |_|  \\__,_|\\__|___/ ";
            std::cout << endl << "                   __/ |";
            std::cout << endl << "                  |___/";
            std::cout << endl << "You correctly guessed the word was " << this->_gmWord->GetWord() << "!";
        }

        void DisplayDefeat() {
            system("CLS");
            std::cout << endl << "      _______";
            std::cout << endl << "     |/      |";
            std::cout << endl << "     |      (_)";
            std::cout << endl << "     |      \\|/";
            std::cout << endl << "     |       |";
            std::cout << endl << "     |      / \\";
            std::cout << endl << "     |";
            std::cout << endl << "  ___|___";
            std::cout << endl;
            std::cout << endl << "   You have lost!";
            std::cout << endl << "   Try harder next time ...";
            std::cout << endl << "   The word was " << this->_gmWord->GetWord();
        }
    };
}

using namespace HangMan;
int main()
{
    HangManGame* hmgGame = new HangManGame();
    hmgGame->StartGameLoop();
}
