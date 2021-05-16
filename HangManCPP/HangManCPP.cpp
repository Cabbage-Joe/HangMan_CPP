/**********************************************************************/
/* This program is a single-player game of hangman. The program will  */
/* prompt the user for the difficulty mode then sudo randomly         */
/* generate  a word to be guessed by the user. If the user can guess  */
/* the generated word while keeping his or her incorrect guesses to   */
/* below a set amount determined by the difficulty, the user will win.*/
/*                                                                    */
/* CPP Compiler: ISO C++14 Standard (/std:c++14)                      */
/* Platform: Windows - This will NOT run on unix based systems        */
/* Title: Hangman                                                     */
/* Author: Joseph M Patchen                                           */
/* GitHub: https://github.com/Cabbage-Joe                             */
/* Date: May 15, 2021                                                 */
/**********************************************************************/

#include <iostream>     /* cout                         */
#include <string>       /* class-string, to_string      */
#include <map>          /* class-map                    */
#include <windows.h>    /* class-HANDLE,GetStdHandle    */
                        /* SetConsoleTextAttribute      */
#include <chrono>       /* milliseconds                 */ 
#include <thread>       /* sleep_for, this_thread       */
#include <iomanip>      /* SetW                         */
#include <random>       /* mt19937, random_device,      */
                        /* uniform_int_distribution     */
#include <stdio.h>      /* getChar                      */    

using namespace std;

namespace HangMan {
    /**********************************************************************/
    /*                         Operator Overloads                         */
    /**********************************************************************/
    #pragma region Operator Overloads
    string operator * (string strEleToBeMultiplied, unsigned int iMult) {  
                                                /* Used primarly for    */
                                                /* spacing              */
        string strRet = "";

        while (iMult--) {
            strRet += strEleToBeMultiplied;
        }
        return strRet;
    }
    #pragma endregion
    /**********************************************************************/
    /*                Symbolic Constants (HangMan Namespace)              */
    /**********************************************************************/
    #pragma region Symbolic Constants
    #define MAX_STRIKES       7             /* Max number of incorrect  */
                                            /* guesses                  */
    #define MAX_SPACE_AFTER_WORD_STATUS 28  /* Total width of the line  */
                                            /* allocated to the word    */
                                            /* status                   */
    #define INCORRECT_GUESS_NUM_EASY    10  /* Max number of incorrect  */
                                            /* guesses for easy mode    */
    #define INCORRECT_GUESS_NUM_MEDIUM  8   /* Max number of incorrect  */
                                            /* guesses for medium mode  */
    #define INCORRECT_GUESS_NUM_HARD    5   /* Max number of incorrect  */
                                            /* guesses for hard mode    */
    #define INCORRECT_GUESS_NUM_INSANE  2   /* Max number of incorrect  */
                                            /* guesses for insane mode  */
    #define NUM_HANGMAN_PARTS           7   /* The number of parts that */
                                            /* make up a full hang man  */
                                            /* graphic                  */
    #define MAX_HANGMAN_HEALTH_WIDTH    24  /* Spaces of a full health  */
                                            /* hangman                  */
    #pragma endregion

    /**********************************************************************/
    /*                           Class-Letter                             */
    /* The letter class will represent elements within a linked list. The */
    /* linked list will represent each word that will be guessed by the   */
    /* user. The letter class will also contain metadata about each       */
    /* specific letter such as if it has been guessed correctly by the    */
    /* user or not.                                                       */
    /**********************************************************************/
    #pragma region Class Letter
    class Letter {
    
    public: 
        char cData;         // The letter 
        bool bIsGuessed;    // Has this letter been guessed
        Letter* lNext;      // Pointer to the next dynmaically
                            // allocated letter in the linked list
    private:
    
    public:
        // Constructor
        Letter(char cDataP, Letter* lNextP) { 
            this->cData = cDataP;
            this->lNext = lNextP;
        }

        // Deconstructor
        ~Letter() {
            // Do nothing
        }

        // Display meta data used for debugging
        void Display() { // Used for debug
            cout << "cData = " << cData << " bIsGuessed = " << bIsGuessed << endl;
        }
    };
    #pragma endregion

    /**********************************************************************/
    /*                          Class-GameWord                            */
    /* The GameWord class is the linked list that contains the word to be */
    /* guessed and which letters have been already guessed correctly by   */
    /* the user.                                                          */
    /**********************************************************************/
    #pragma region Class GameWord
    class GameWord {

    private:
        Letter* _lHead; // Head of the linked list
        Letter* _lEnd; // End node of the linked list

    public:
        bool bWordHasBeenGuessed = false; // Has every letter of the word been guessed

        // Constructor
        GameWord() {
            // Do nothing
        }
        
        // Deconstructor - Unalloacate linked list Letter objects stored on the heap
        ~GameWord() {
            // Unallocated Letter objects stored on the heap
            // Must be done to prevent memory leaks
            Letter* lEle = _lHead;
            do {
                Letter* lEleToBeDeleted = lEle;
                lEle = lEle->lNext;
                delete lEleToBeDeleted;
            } while (lEle != NULL);
        }

        // Get the linked list word as a string (copy)
        string GetWord() {
            string strRet = "";

            Letter* lEle = _lHead;
            do {
                strRet += lEle->cData;
                lEle = lEle->lNext;
            } while (lEle != NULL);

            return strRet;
        }

        // Get a string representing which letters have been guessed correctly
        string GetWordStatus() {
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

        // Create the linked list from the word to be guessed
        void SetUpLinkedList(string strGameWord) {
            bool bFirstPass = true;
            for (auto cEle : strGameWord)
            {
                if (bFirstPass) {
                    // Create the head - must use the new keyword so
                    // the elements of the linked list will 
                    // be stored on the heap
                    // Note: we must free these elements to prevent memory leaks 
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

        // Guess a letter in the game word
        // If the guess is correct update meta data in the linked list
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
        // Print debugging info for the linked list
        void PrintListContents() { // For debugging
            Letter* lEle = _lHead;
            do {
                lEle->Display();
                lEle = lEle->lNext;
            } while (lEle != NULL);
            std::cout << endl;
        }
    };
    #pragma endregion

    /**********************************************************************/
    /*                         Class-HangManGame                          */
    /* The HangManGame class is the main structure that is the game       */
    /* itself. This class contains the linked list of the word to be      */
    /* guessed and other metadata needed for the game. The class contains */
    /* a StartGameLoop method that is the entry point for the game        */
    /* itself. The games output is stdout                                 */
    /**********************************************************************/
    #pragma region Class HangManGame
    class HangManGame {
        enum class Difficulty
        {
            Easy = 1,
            Medium = 2,
            Hard = 3,
            Insane = 4
        };

    private:
        HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Handle for windows environment console
                                                            // (tested on Windows 10) which is used 
                                                            // for changing the color of foreground
                                                            // and background of the console.
        GameWord _gmWord;   // Linked List that is main data structure of the game
                            // This object will be stored on the stack
        int _iIncorrectGuesses = 0; //Number of incorrect guesses by the user
        map<int,string> _mWordsToBeGuessed = {  {0, "abruptly",}, 
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
                                                {16, "marquis",},}; // Dictionary that contains the words that will randomly be selected for the user to guess
    
    public:
        // Constructor
        HangManGame() {
            // Do Nothing
        }

        // Deconstructor
        ~HangManGame() {
        
        }

        // Main entry point for the HangMan game
        void StartGameLoop() {
            char cPlayAgainAnswer;

            do {
                string strGameWord = "";

                // Display main screen
                int iMaxIncorrectGuess;
                Difficulty dGameMode = DisplayIntroAndGetDifficulty();
                iMaxIncorrectGuess = GetIncorrectGuessAmount(dGameMode);
                std::cout << endl << "You will have " << iMaxIncorrectGuess << " incorrect guesses before you lose the game";
                std::cout << endl << "Good luck!";

                // Generate word to be guessed from the dictionary
                std::cout << endl  << endl << "Generating a random word for you to guess ...";
                int iWordDictionarySize = _mWordsToBeGuessed.size();
                std::random_device rdSeed;  // The seed
                std::mt19937 mtEngine(rdSeed());   // mersenne twister with seed
                std::uniform_int_distribution<> uidDisto(0, iWordDictionarySize - 1);
                int iRandomNumber = uidDisto(mtEngine);
                strGameWord = _mWordsToBeGuessed[iRandomNumber];
                std::cout << endl << endl;
                int iSeccondWaitTime = 6;
                for (int index = 0; index < iSeccondWaitTime; index++) {
                    std::cout << endl << "Starting in " << std::to_string(iSeccondWaitTime - index) << " seccond(s)";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }

                // Set up linked list
                _gmWord.SetUpLinkedList(strGameWord);

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
                        bWon = _gmWord.bWordHasBeenGuessed;
                        bGuessIsCorrect = _gmWord.Guess(cGuess);
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
                bool bFirstPass = true;
                std::cout << endl << endl << "Would you like to play again(y/n)?";
                do {
                    if (!bFirstPass) {
                        std::cout << endl << "Please enter a valid answer (y/n):";
                    }
                    if (bFirstPass) {
                        bFirstPass = false;
                    }
                    cPlayAgainAnswer = GetExactlyOneCharFromStdin();
                } while (cPlayAgainAnswer != 'n' && cPlayAgainAnswer != 'y');
            } while (cPlayAgainAnswer == 'y');   
        }

        // Get only one char from stdin and throw out anything extra (message user if throw out occurs)
        char GetExactlyOneCharFromStdin() {
            char cRet;
            bool bFirstPass = true;
            string strNotUsed = "";
            char cNew;
            do {
                cNew = tolower(getchar());
                if (bFirstPass) {
                    cRet = cNew;
                    bFirstPass = false;
                }
                else if (cNew !='\n') {
                    strNotUsed += cNew;
                }
            } while (cNew != '\n');

            // Display message if user entered more than one char
            if (strNotUsed.compare("")) {
                std::cout << endl << "Using: " << cRet;
                std::cout << endl << "Not using: "<< strNotUsed;
                std::cout << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }

            return cRet;
        }

        // Display main intro screen (HangMan title)
        // and gets which difficulty mode from the user via stdin
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

            char cDifficulty;
            int iDifficulty;
            cDifficulty = GetExactlyOneCharFromStdin();
            iDifficulty = cDifficulty - 48;

            dRet = static_cast<Difficulty>(iDifficulty);

            return dRet;
        }

        // Determines how many incorrect guesses the user will 
        // have depending upon the selected difficulty
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

        // Determines the of the health bar depending upon 
        // how many incorrect guesses have been made by the user
        void SetHealthBarColor(double dIncorrectGuessToMaxRatio) {
            // Hang man still has at least 50% health
            if (dIncorrectGuessToMaxRatio <= 0.50) { 
                SetConsoleTextAttribute(_hConsole, FOREGROUND_GREEN | BACKGROUND_GREEN);
            }
            // Hang man still has at least 75% health
            else if (dIncorrectGuessToMaxRatio <= 0.75) {
                SetConsoleTextAttribute(_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_GREEN);
            }
            // Hang man still has at least 25% health
            else {
                SetConsoleTextAttribute(_hConsole, FOREGROUND_RED | BACKGROUND_RED);
            }
        }

        // Resets the console back to standard black and white colors
        void ResetColorToDefault() {
            SetConsoleTextAttribute(_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        }

        // Displays the current progress of the hangman being hanged and gets the next guess from the user
        char DisplayHangManStatusAndGetNextGuess(int iCurrentIncorrectGuesses,int iMaxIncorrectGuesses, GameWord& gmWord) {
            char cRet;
            string strWordStatus = gmWord.GetWordStatus();
            string strSpace = " ";
            int iWordStatusLength = strWordStatus.length();

            // Get progress of hangman
            double dIncorrectGuessToMaxRatio = ((double)iCurrentIncorrectGuesses / iMaxIncorrectGuesses);
            int iHangManPartsProgress = (int)(dIncorrectGuessToMaxRatio * NUM_HANGMAN_PARTS);
            int iHangManHealthSubtract = (int)(dIncorrectGuessToMaxRatio * MAX_HANGMAN_HEALTH_WIDTH);
            int iHangManHealth = MAX_HANGMAN_HEALTH_WIDTH - iHangManHealthSubtract;
            short iHangManEmptyHealthPortionSize = (short)MAX_HANGMAN_HEALTH_WIDTH - (short)iHangManHealth;

            // Display hang man status header
            ios init(NULL);
            string strHealthEleSymbol = "=";
            init.copyfmt(cout); // Save orginal formatting
            system("CLS"); 
            std::cout << endl << "=======================================================";
            std::cout << endl << "= ";
            std::cout << std::left << setw(20)                          << "Incorrect Guess: " + std::to_string(iCurrentIncorrectGuesses);
            std::cout << std::left << setw(7)                           << "Health ";
            SetHealthBarColor(dIncorrectGuessToMaxRatio);
            std::cout << std::left << setw(iHangManHealth) << strHealthEleSymbol * iHangManHealth << std::flush;
            ResetColorToDefault();
            std::cout << std::right << setw(iHangManEmptyHealthPortionSize + (short)2) << " =";
            cout.copyfmt(init); // restore default formatting
            std::cout << endl << "=                   WORD: " + gmWord.GetWordStatus() + (strSpace * (MAX_SPACE_AFTER_WORD_STATUS - iWordStatusLength)) + "=";
            

            // Display graphic lines 1 and 2
            std::cout << endl << "=                       _______                       =";
            std::cout << endl << "=                      |/      |                      =";

            // Display graphic line 3
            if (iHangManPartsProgress >= 1) {
                std::cout << endl << "=                      |      (_)                     =";
            }
            else {
                std::cout << endl << "=                      |                              =";
            }

            // Displat graphic line 4
            if (iHangManPartsProgress >= 4) {
                std::cout << endl << "=                      |      \\|/                     =";
            }
            else if (iHangManPartsProgress >= 3){
                std::cout << endl << "=                      |      \\|                      =";
            } 
            else if (iHangManPartsProgress >= 2) {
                std::cout << endl << "=                      |      \\                       =";
            }
            else {
                std::cout << endl << "=                      |                              =";
            }

            // Display graphic line 5
            if (iHangManPartsProgress >= 5) {
                std::cout << endl << "=                      |       |                      =";
            }
            else {
                std::cout << endl << "=                      |                              =";
            }

            // Display graphic line 6
            if (iHangManPartsProgress >= 6) {
                std::cout << endl << "=                      |      /                       =";
            }
            else {
                std::cout << endl << "=                      |                              =";
            }
            
            // Display graphic lines 7 and 8
            std::cout << endl << "=                      |                              =";
            std::cout << endl << "=                   ___|___                           =";

            // Get the next guess
            std::cout << endl << "=======================================================";
            std::cout << endl << endl << "Please enter in a guess: ";
            cRet = GetExactlyOneCharFromStdin();

            return cRet;
        }    

        // Displays the congratulations message if the user has won 
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
            std::cout << endl << "You correctly guessed the word was " << this->_gmWord.GetWord() << "!";
        }

        // Displays the defeat message if the user has lost
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
            std::cout << endl << "   The word was " << this->_gmWord.GetWord();
        }
    };
    #pragma endregion
}

using namespace HangMan;

int main()
{
    HangMan::HangManGame hmgGame; // Will be stored on the stack
    hmgGame.StartGameLoop();
}
