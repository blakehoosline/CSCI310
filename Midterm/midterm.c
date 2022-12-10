#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

const bool test = false;
                        
#define NUM_THREADS 2

struct Robot {
  int pos_x;
  int pos_y;

  int n_goldcollected;
};

struct GoldBar {
  int pos_x;
  int pos_y;
  bool available;
};

struct Bomb {
  int pos_x;
  int pos_y;
};

struct Workspace {
  char pos[4][4];
  int n_gold, turnCount;
  bool robot_caught;

  struct Robot wall_e;
  struct GoldBar gb1;
  struct GoldBar gb2;

  struct Bomb bmb;
};

struct thread_data{
  pthread_t thread_id;
  int thread_num;
  int move_count;
  struct Workspace* map;
}thread_data_array[NUM_THREADS];

struct timespec sleeptime  = {0, 5000};
pthread_mutex_t map_mutex;


int getRandom(int rangeLow, int rangeHigh);
void randPos(struct Workspace *map);
void createWorld(struct Workspace *map);
void updateWorkspace(struct Workspace *map);
bool MapHasGold(struct Workspace *map);
void randomMove(int *x, int *y);
bool hasBomb(struct Workspace *map, int x, int y);
bool isValidMove(struct Workspace *map, int x, int y, void* threadarg);
void checkNextSquare(struct Workspace *map, void* threadarg);
void moveNext(struct Workspace *map, const int x, const int y, void* threadarg);
bool hasGold(struct Workspace *map, int x, int y);
void getGold(struct Workspace *map);
void* Run4Gold(void* threadarg);
void* Run4Robot(void* threadarg);
void printMap(struct Workspace *map);
void init(struct Workspace *map);
void wizard(struct Workspace *map);
void *bombAPI(void*threadarg);
void *robotAPI(void *threadarg);
void run(void* threaddata);
bool foundGold(struct Workspace *map);
void init_thread_data(struct thread_data* thread, struct Workspace* map);
void StartAPI();


int getRandom(int rangeLow, int rangeHigh) {
  double myRand = rand() / (1.0 + RAND_MAX);
  int range = rangeHigh - rangeLow + 1;
  int myRand_scaled = (myRand * range) + rangeLow;
  return myRand_scaled;
} 


void randPos(struct Workspace *map) {
  int x, y, result, low = 0, high = 15, place[4] = {-1};
  bool similar;

  srandom(time(NULL));

  for (x = 0; x < 4; x++) {
    do {
      similar = false;
      result = getRandom(low, high);
      for (y = 0; y < 4; y++) {
        if (result == place[y]) {
          similar = true;
        }
      }
    } while (similar);
    place[x] = result;
  }
 
  x = place[0] / 4;
  y = place[0] % 4;
  map->wall_e.pos_x = x;
  map->wall_e.pos_y = y;
  map->pos[x][y] = 'R';

  x = place[1] / 4;
  y = place[1] % 4;
  map->bmb.pos_x = x;
  map->bmb.pos_y = y;
  map->pos[x][y] = 'B';

  x = place[2] / 4;
  y = place[2] % 4;
  map->gb1.pos_x = x;
  map->gb1.pos_y = y;
  map->pos[x][y] = 'G';

  x = place[3] / 4;
  y = place[3] % 4;
  map->gb2.pos_x = x;
  map->gb2.pos_y = y;
  map->pos[x][y] = 'G';

} 
void createWorld(struct Workspace *map) {

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      map->pos[i][j] = '-';
    }
  }

} 
void updateWorkspace(struct Workspace *map) {

  createWorld(map);
  if (map->gb1.available) {
    map->pos[map->gb1.pos_x][map->gb1.pos_y] = 'G';
  }
  if (map->gb2.available) {
    map->pos[map->gb2.pos_x][map->gb2.pos_y] = 'G';
  }
  map->pos[map->wall_e.pos_x][map->wall_e.pos_y] = 'R';

  map->pos[map->bmb.pos_x][map->bmb.pos_y] = 'B';
}
bool MapHasGold(struct Workspace *map) {
  printf("Map has %d gold bar(s).\n", map->n_gold);
  return (map->n_gold > 0);
}
void randomMove(int *x, int *y) {
  do {
    *x = getRandom(0, 2) - 1;
    *y = getRandom(0, 2) - 1;
  } while ((*x == 0) && (*y == 0));
}
bool hasBomb(struct Workspace *map, int x, int y) {
  bool bmb = false;
  if (map->pos[x][y] == 'B') {
    bmb = true;
    printf("Bomb has been found, not moving.\n");
  }
  return bmb;
}
bool hasGold(struct Workspace *map, int x, int y) {
  bool gold = false;
  if (map->pos[x][y] == 'G') {
    gold = true;
    printf("Bomb has been found, not moving.\n");
  }
  return gold;
}
bool isValidMove(struct Workspace *map, int x, int y, void* threadarg) {
  bool valid;
  struct thread_data* t_dat = (struct thread_data*) threadarg;

if (t_dat->thread_num == 0){
  if ((map->wall_e.pos_x + x >= 4) || (map->wall_e.pos_x + x) < 0) {
    valid = false;
  }
  else if ((map->wall_e.pos_y + y >= 4) || (map->wall_e.pos_y + y < 0)) {
    valid = false;
  }
  else if (hasBomb(map, map->wall_e.pos_x + x, map->wall_e.pos_y + y)) {
    valid = false;
  }
  else {
    valid = true;
    printf("Safe to move.\n");
  }
}
else if (t_dat->thread_num == 1){
  if ((map->bmb.pos_x + x >= 4) || (map->bmb.pos_x + x) < 0) {
    valid = false;
  }
  else if ((map->bmb.pos_y + y >= 4) || (map->bmb.pos_y + y < 0)) {
    valid = false;
  }
  else if (hasGold(map, map->bmb.pos_x + x, map->bmb.pos_y + y)) {
    valid = false;
  }
  else {
    valid = true;
    printf("Safe to move.\n");
  }
}
  return valid;
}
void checkNextSquare(struct Workspace *map, void* threadarg) {
  int x, y;
  while (true) {
    x = y = 0;
    randomMove(&x, &y);
    printf("Checking square (%d,%d).\n", map->wall_e.pos_x + x + 1, map->wall_e.pos_y + y + 1);
    if (isValidMove(map, x, y, threadarg)) {
      break;
    }
  }
  moveNext(map, x, y, threadarg);
}
void moveNext(struct Workspace *map, const int x, const int y, void* threadarg) {

  struct thread_data* t_dat = (struct thread_data*) threadarg;

  if (t_dat->thread_num == 0){
	map->wall_e.pos_x = map->wall_e.pos_x + x;
  	map->wall_e.pos_y = map->wall_e.pos_y + y;
    printf("Robot is now at (%d,%d)\n", map->wall_e.pos_x + 1, map->wall_e.pos_y + 1);
  }
  else if (t_dat->thread_num == 1){
	map->bmb.pos_x = map->bmb.pos_x + x;
	map->bmb.pos_y = map->bmb.pos_y + y;
	printf("Bomb is now at (%d,%d)\n", map->bmb.pos_x + 1, map->bmb.pos_y + 1);
  }
} 
bool foundGold(struct Workspace *map) {
  printf("Checking if gold is encountered.\n");

  updateWorkspace(map);
  int pos_x = map->wall_e.pos_x;
  int pos_y = map->wall_e.pos_y;

  if ((map->gb1.pos_x == pos_x) && (map->gb1.pos_y == pos_y) &&
      map->gb1.available) {
    printf("Found gold!\n");
    return 1;
  }
  if ((map->gb2.pos_x == pos_x) && (map->gb2.pos_y == pos_y) &&
      map->gb2.available) {
    printf("Found gold!\n");
    return 1;
  }

  printf("Gold not found.\n");
  return 0;
}
void getGold(struct Workspace *map) {

  if ((map->gb1.pos_x == map->wall_e.pos_x) &&
      (map->gb1.pos_y == map->wall_e.pos_y) && map->gb1.available) {
    map->gb1.available = false;
    map->n_gold--;
    map->wall_e.n_goldcollected++;
  }
  if ((map->gb2.pos_x == map->wall_e.pos_x) &&
      (map->gb2.pos_y == map->wall_e.pos_y) && map->gb2.available) {
    map->gb2.available = false;
    map->n_gold--;
    map->wall_e.n_goldcollected++;
  }
  printf("Gold collected!\n");
}
void *Run4Gold(void* threadarg) {
  struct thread_data* t_dat = (struct thread_data*) threadarg;
  while (1) {
    pthread_mutex_lock(&map_mutex);
    if (!(MapHasGold(t_dat->map))) {break;}
    if (t_dat->map->robot_caught) {
	 pthread_mutex_unlock(&map_mutex);
	 pthread_exit(NULL);}


    t_dat->map->turnCount++;

    checkNextSquare(t_dat->map, threadarg);

    if (foundGold(t_dat->map)) {
      getGold(t_dat->map);
    }

    printMap(t_dat->map);
    pthread_mutex_unlock(&map_mutex);
    nanosleep(&sleeptime,NULL);
  }
  pthread_mutex_unlock(&map_mutex);
  printf("All gold has been collected.\n");
  pthread_exit(NULL);
} 
void *Run4Robot(void* threadarg) {
   struct thread_data* t_dat = (struct thread_data*) threadarg;
  while (1) {
    pthread_mutex_lock(&map_mutex);
    if (t_dat->map->robot_caught) { break;}
    if (!MapHasGold(t_dat->map)) { 
	printf("Exiting bomb thread.\n");    
	pthread_mutex_unlock(&map_mutex);
	pthread_exit(NULL);}

    t_dat->map->turnCount++;

    checkNextSquare(t_dat->map,threadarg);

    if ((t_dat->map->bmb.pos_x == t_dat->map->wall_e.pos_x) && (t_dat->map->bmb.pos_y == t_dat->map->wall_e.pos_y)){
	t_dat->map->robot_caught = true;
    }	
    printMap(t_dat->map);
    pthread_mutex_unlock(&map_mutex);
    nanosleep(&sleeptime, NULL);
  }

  pthread_mutex_unlock(&map_mutex);

  printf("Robot has been blown up!\n");
  pthread_exit(NULL);

}
void printMap(struct Workspace *map) {

  updateWorkspace(map);
  int i;

  printf("\nTurn: %i\n", map->turnCount);
  for (i = 0; i < 4; i++) {
    printf("-----------------\n| %c | %c | %c | %c |\n", map->pos[0][i],
           map->pos[1][i], map->pos[2][i], map->pos[3][i]);
  }
  printf("-----------------\n");
}
void init(struct Workspace *map) {
  createWorld(map);
  printf("Initializing to random position");
  randPos(map);

  map->n_gold = 2;
  
  map->wall_e.n_goldcollected = 0;
  
  map->gb1.available = true;
  map->gb2.available = true;

  map->turnCount = 0;

  map->robot_caught = false;
}
void init_thread_data(struct thread_data* thread, struct Workspace* map)
{
	thread[0].thread_num = 0;
	thread[1].thread_num = 1;

	thread[0].move_count = 0;
	thread[1].move_count = 0;

	thread[0].map = map;
	thread[1].map = map;
}
void run(void* threaddata)
{
	pthread_mutex_lock(&map_mutex);

	struct thread_data *tdat = (struct thread_data*) threaddata;

	if (tdat-> thread_num == 0) { 
		
		Run4Gold(threaddata);
	}

	if (tdat-> thread_num == 1) {
		Run4Robot(threaddata);
	}

	pthread_mutex_unlock(&map_mutex);
}
void wizard(struct Workspace *map) {
    int turnCount;
    if (turnCount % 5 == 0) {
        printf("Randomizing positions");
        randPos(map);

            map->n_gold;
        
        map->wall_e.n_goldcollected;
        
        map->gb1.available;
        map->gb2.available;

        map->turnCount;

        map->robot_caught;
    } 
}
void *bombAPI(void*threadarg)
{
	run(threadarg);
}
void *robotAPI(void *threadarg){
	run(threadarg);
}
void StartAPI(){
  struct Workspace map;
  init(&map);
  printMap(&map);

  init_thread_data(thread_data_array, &map);
  
  pthread_mutex_init(&map_mutex, NULL);
  pthread_create(&(thread_data_array[0].thread_id), NULL, Run4Gold, (void *)(&thread_data_array[0]));
  pthread_create(&(thread_data_array[1].thread_id), NULL, Run4Robot, (void *)(&thread_data_array[1]));

  pthread_join(thread_data_array[0].thread_id,NULL);
  pthread_join(thread_data_array[1].thread_id,NULL);
  
  pthread_mutex_destroy(&map_mutex); 

}

int main() {

  StartAPI();

  printf("\nExiting program...\n");

  return 0;
}
