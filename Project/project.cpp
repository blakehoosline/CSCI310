#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <mutex>
using namespace std;

const int NUM_THREADS = 2;
const int NUM_ROWS = 3;
const int NUM_COLS = 3;

char board[NUM_ROWS][NUM_COLS] = {{' ', ' ', ' '},

                                {' ', ' ', ' '},

                                 {' ', ' ', ' '}};

void *play_game(void *threadid);
void print_board();
bool check_win();

std::mutex mtx; 


int main()
{
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_t threads[NUM_THREADS];
    int rc;
    int i;

    for (i = 0; i < NUM_THREADS; i++)
    {
        mtx.lock();
        cout << "Main: creating thread " << 0 << endl;
        rc = pthread_create(&threads[i], &attr, play_game, NULL);
        mtx.unlock();
        if (rc)
        {
            cout << "Error: unable to create thread " << rc << endl;
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        rc = pthread_join(threads[i], NULL);
        if (rc)
        {
            cout << "Error: unable to join thread " << rc << endl;
            exit(-1);
        }
        cout << "Main: completed thread id :" << i << endl;
    }
   pthread_attr_destroy(&attr);
   pthread_mutex_destroy(&mutex);
   pthread_exit(NULL);
}


void *play_game(void *threadid)
{
    srand(time(NULL));
    long tid = (long)threadid;
    while(check_win() != true){
        
        if (tid == 0)
        {
            int row = rand() % NUM_ROWS;
            int col = rand() % NUM_COLS;
    
           board[row][col] = 'X';
        }
    
        print_board();
    
        if (check_win())
        {
           pthread_exit(NULL);
        }
    
        int row = rand() % NUM_ROWS;
        int col = rand() % NUM_COLS;
    
        board[row][col] = 'O';
    
        print_board();
    
        if (check_win())
        {
           pthread_exit(NULL);
        }
        
    }
   pthread_exit(NULL);
}

void print_board()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            cout << board[i][j] << "|";
        }
        cout << endl;
    }
    cout << endl;
    cout <<"--------------"<<endl;
}

bool check_win()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        if (board[i][0] != ' ' &&
           board[i][0] == board[i][1] &&
           board[i][1] == board[i][2])
        {
            return true;
        }
    }
    
    for (int i = 0; i < NUM_COLS; i++)
    {
        if (board[0][i] != ' ' &&
           board[0][i] == board[1][i] &&
           board[1][i] == board[2][i])
        {
            return true;
        }
    }

    if (board[0][0] != ' ' &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2])
    {
        return true;
    }

    if (board[0][2] != ' ' &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0])
    {
        return true;
    }

    return false;
}
