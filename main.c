#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int MazeGenerate(int* maze, int dim);
void MazePrint(int* maze, int dim);
void PrintBorder(char* str, int* maze, int dim, int row);
void PrintRow(char* str, int* maze, int dim, int row);
int randGap(int, int);
int chooseBorder(int dim);
int mazeChannel(int* maze, const int dim, const int source);
int cellNeighbor(const int* maze, const int dim, const int cell, const int skip, const int pattern);
int cellBorder(const int dim, int cell);
int ChannelCheck(int* maze, const int dim, const int source);
int playerWalk(int* maze, const int dim, int* players, const int players_N, const int player);
int printPlayer(const int *maze, const int dim, int *players, const int player, const int viewed_R);
int shoot(const int *maze, const int dim, int *players, int players_N, int curr_cell, int dir);
int PlaceWall(int *maze, const int dim, const int cell, const int dir_a);
void PlaceExit(int* maze, const int dim);
void PlaceHoles(int* maze, const int dim, const int holes_N);
int CellsAvailable(int* maze, const int dim, const int start);

int main(){
//seed rand()
    srand(time(NULL));
    int N;
    const int players_N = 1;
    printf("size: ");
    scanf("%d", &N);
    printf("\n");
    int *maze = (int *)malloc(N*(N+1)*11 * sizeof(int));
    int *players = (int *)malloc(players_N*5 * sizeof(int));
    MazeGenerate(maze, N);
//    MazePrint(maze, N);
//    MazePrint(maze);
    players[0] = 13;
    players[1] = 3;
    players[2] = 3;
    players[3] = 3;
    players[4] = 0;
    while(0 == playerWalk(maze, N, players, players_N, 0)){
    }




    free(maze);
    free(players);
    return 0;
}
/*
cell:
["description", up, right, down, left, on_enter, w_up, w_right, w_down, w_left, items]
maze example:
[ , 'R', 2, 6, 8, 4, 3, 0, 0, 0, 0, 0, 'R', 3, -1, 9, 5, 3, 0, 0, 0, 0, 0,  ]
     |                              |   |                               |
     +-----------cell № 5-----------+   +-----------cell № 6------------+

view:
+-+-+-+
|r| |h|
+ + + +
|r e|e 
+ + + +
|d| |h|
+-+-+-+ 


+-+-+-+
|1|2|3|
+ + + +
|4|5|6| 
+ + + +
|7|8|9|
+-+-+-+ 

player:
[, cell, lives, bullets, bombs, treasure, ]

DONE: fix bugs with sings of walls in MazePrint
DONE: add adequate random walls
DONE: make outer walls undestroyble
DONE: add bullet
DONE: add dispose of bombs, bullets, lives
DONE: add/fix exit room (room № 0)
DONE: add holes generation
TODO: add "map is winable" checkout
TODO: rewrite all functions prototypes from "int" to "const int"
TODO: add multiplayer
TODO: check bullet
TODO: add socket interface support

TODO: fix the following bugs:
BUG: when exit is at cell 25, it breaks the printing of the last row on the screen


FIXED:
BUG: when walking 'R' -> 'R' it works like when player walks from ' ' -> 'R'

*/

int MazeGenerate(int* maze, int dim){
    //cell 0 -- the exit
    //cell -1 -- dummy cell
    
    //geometry of maze
    for(int i = 1; i <= dim*dim; ++i){
        //walls
        maze[i*11 + 6] = 0;
        maze[i*11 + 7] = 0;
        maze[i*11 + 8] = 0;
        maze[i*11 + 9] = 0;

        maze[i*11] = ' ';//empty sell

        if ((i - dim) <= 0){ //up
            maze[i*11 + 1] = -1;
            maze[i*11 + 6] = 1;
        }else{
            maze[i*11 + 1] = i - dim;
        }
        if (i % dim == 0){//right
            maze[i*11 + 2] = -1;
            maze[i*11 + 7] = 1;
        }else{
            maze[i*11 + 2] = i + 1;
        }
        if ((i + dim) > (dim * dim)){//down
            maze[i*11 + 3] = -1;
            maze[i*11 + 8] = 1;
        }else{
            maze[i*11 + 3] = i + dim;
        }
        if (i % dim == 1){//left
            maze[i*11 + 4] = -1;
            maze[i*11 + 9] = 1;
        }else{
            maze[i*11 + 4] = i - 1;
        }
        maze[i*11 + 5] = i; //no holes or rivers => teleport to itself
    }
    //cell №0 -- exit cell
    
    
  //generate river
  //choose source and drain cells
  int source = chooseBorder(dim);
//  int source = 23;
//  printf("S %d\n", source);
  maze[source*11] = 'R';
  mazeChannel(maze, dim, source);
  //place hospital
  int hospital;
  do{
    hospital = randGap(1, dim*dim);
  }while(maze[11*hospital] != ' ');
  maze[11*hospital] = 'H';
  //place armory
  int armory;
  do{
    armory = randGap(1, dim*dim);
  }while(maze[11*armory] != ' ');
  maze[11*armory] = 'W';
//place random walls
  int walls_total = dim;
  do{
    if(!PlaceWall(maze, dim, randGap(1, dim*dim), randGap(1, 4))){
      ++walls_total;
    }
  }while(--walls_total);
  
  PlaceExit(maze, dim);
  int holes_N = dim;
  PlaceHoles(maze, dim, holes_N);

// print maze as table. Useful for debug  
/*
  for (int cell = 0; cell <= dim*dim; ++cell){
    for(int i = 0; i < 11; ++i){
      printf(" %3d ", maze[cell*11 + i]);
    }
    printf("\n");
  }
*/


  MazePrint(maze, dim);
  return 0;
}

//["description", up, right, down, left, on_enter, w_up, w_right, w_down, w_left, items]





void PlaceHoles(int* maze, const int dim, const int holes_N){
  int* holes = (int *)malloc(holes_N * sizeof(int));

  int i = 0;
  int cell = 0;
  printf("generating holes\n");
  while (i < holes_N){
    cell = randGap(1, dim*dim);
    if (maze[11*cell] == ' '){
      printf("placed hole at %d\n", cell);
      maze[11*cell] = '0';
      holes[i] = cell;
      ++i;
    }
  }

  i = 0;
  for (int j = 0; j < holes_N; ++j){
    int src = holes[j];
    int dest = holes[randGap(0, holes_N - 1)];
    maze[11*src + 5] = dest; 
    printf("connected hole at %d to %d\n", src, dest);
  }
  printf("\n");

/*
  while (i < holes_N){
    cell = randGap(1, dim*dim);
    if (maze[11*cell] == '0'){
      int hole_dest = holes[randGap(1, holes_N) -1];
      printf("connected hole at %d to %d\n", cell, hole_dest);
      maze[11*cell + 5] = hole_dest;
      printf("%d\n", i);
      ++i;
    }
  }
*/
  free(holes);

}
void PlaceExit(int* maze, const int dim){
  int exit = chooseBorder(dim);
//  exit = randGap(1, dim);
  printf("exit near cell %d\n", exit);
  int dir = 0;
  for(int i = 1; i <= 4; ++i){
    if (maze[11*exit + i] == -1){
      if (dir != 0){
        dir = i;
      }else{
        if (randGap(0,1)){
          dir = i;
        }
      }
    }
  }
  maze[11*exit + dir] = 0;
  maze[11*exit + 5 + dir] = 0;

  maze[0] = 'E';
  maze[1] = exit;
  maze[2] = exit;
  maze[3] = exit;
  maze[4] = exit;
  maze[5] = 0;
  maze[6] = 0;
  maze[7] = 0;
  maze[8] = 0;
  maze[9] = 0;
  maze[10] = 0;
  

}

void MazePrint(int* maze, int dim){
    char *str = (char *)malloc((2*dim + 1)*sizeof(char));
    for(int i = 0; i < dim; ++i){
        PrintBorder(str, maze, dim, i);
        PrintRow(str, maze, dim, i);
    }
    PrintBorder(str, maze, dim, dim);
//    borderPrint(int* maze, int dim, int i);
    
}

void PrintBorder(char* str, int* maze, int dim, int row){
    if(row == dim){
//    if(0){
      for(int i = 0; i < dim; ++i){
        str[2*i] = '+';
        if(maze[((row-1)*(dim) + i+1)*11 + 8] == 1){
          str[2*i + 1] = '-';
        }else{
          str[2*i + 1] = ' ';
        }
      }
    }else{
      for(int i = 0; i < dim; ++i){
        str[2*i] = '+';
        if(maze[(row*(dim) + i+1)*11 + 6] == 1){
          str[2*i + 1] = '-';
        }else{
          str[2*i + 1] = ' ';
        }
      }
    }
    str[2*dim] = '+';
    printf("%s\n", str);
}

void PrintRow(char* str, int* maze, int dim, int row){
//draw walls
    for(int i = 1; i <= dim; ++i){
        if(maze[(row*dim + i)*11 + 9] == 1){
            str[2*(i-1)] = '|';
        }else{
            str[2*(i-1)] = ' ';
        }
    }
    if(maze[(row*dim+dim)*11 + 7] == 1){
        str[2*dim] = '|';
    }else{
        str[2*dim] = ' ';
    }
//draw fillers
    for(int i = 1; i <= dim; ++i){
        str[2*(i-1) +1] = maze[11*(i + row*dim)];
    }
    
    printf("%s\n", str);
    
// show teleport destination for debug purposes
/*
    for(int i = 1; i <= dim; ++i){
        printf("%2d", maze[11*(i + row*dim) + 5]);
    }
    printf("\n");
*/
}

int randGap(int a, int b){//return random int in [a;b]
    double p = (double)rand()/((double)RAND_MAX);
//    printf("%g\n",p);
    int c = round(p*(b - a) + a );
//    printf("    %d\n", c);
    return c;
}


/*
int chooseBorder(int dim){
    int total = 4*dim - 4;
    int choosed = randGap(1, total);
    printf("total %d\n choosed %d\n", total, choosed);
    if(choosed <= dim + 1){
        return choosed;
    }
    if(choosed >= total - dim -1){
        return (1 + dim*dim) - (total - choosed);
    }
    choosed -= dim -2;
    if(dim % 2 == 0){
        choosed /=2;
        return choosed*dim;
    }else{
        choosed /=2;
        return choosed*dim -1;
    }
}
*/


int chooseBorder(int dim){
    int choosed = randGap(1, dim*dim);
    while(!cellBorder(dim, choosed)){
        choosed = randGap(1, dim*dim);
    }
    return choosed;
}


int mazeChannel(int* maze, const int dim, const int source){
    int curr = source;
    int len = 0;
    int* maze1 = (int *)malloc(dim*(dim+1)*11 * sizeof(int));
    for(int i = 0; i < dim*(dim+1)*11; ++i){
        maze1[i] = maze[i];
    }
    while (ChannelCheck(maze1, dim, source) < dim){ //here you can set the min and max length of river
        for(int i = 0; i < dim*(dim+1)*11; ++i){
            maze1[i] = maze[i];
        }

        int i = dim*dim/2;
        int prohib_direction, direction;
        curr = source;
        while (i){
            --i;
            prohib_direction = direction + 2;
            if(prohib_direction > 4){
                prohib_direction -= 4;
            }
            while(1){
                direction = randGap(1, 4); //get the number of the next cell
                if (prohib_direction != direction){
                    break;
                }
            }
            int next = maze1[11*curr + direction];
//            printf("%d ",next);
            if (next == -1){
                continue;
            }
            if ((! cellNeighbor(maze1, dim, next, curr, 'R') && next > 0)){
                maze1[11*next] = 'R';
                curr = next;
            }
            if ((! cellNeighbor(maze1, dim, next, curr, 'R') && next <= 0)){
                if (randGap(1,2) == 1){
                    maze1[11*next] = 'D';
                    break;
                }else{
                    maze1[11*next] = 'R';
                    curr = next;
                }
            }
//            MazePrint(maze1, dim);
        }
    }
    for(int i = 0; i < dim*(dim+1)*11; ++i){
        maze[i] = maze1[i];
    }
}

int cellNeighbor(const int* maze, const int dim, const int cell, const int skip, const int pattern){
    for(int i = 1; i <= 4; ++i){
        if(maze[cell*11 + i] != -1){
//            printf("neghbor %d; cell %d; dir %d\n", maze[cell*11 + i], cell, i);
            if(maze[11*maze[cell*11 + i]] == pattern){
                if(maze[11*cell+i] != skip){
                    return i;
                }
            }
        }
    }
    return 0;
}

int cellBorder(const int dim, int cell){
    int choosed = 0;
    if ((cell - dim) <= 0){ //up
        choosed = 1;
    }
    if (cell % dim == 0){//right
        choosed = 2;
    }
    if ((cell + dim) > (dim * dim)){//down
        choosed = 3;
    }
    if (cell % dim == 1){//left
        choosed = 4;
    }
    return choosed;
}

int ChannelCheck(int* maze, const int dim, const int source){
    int curr = source;
    int next = 0;
    int prev1 = source, prev2 = source;
    int river_len = 0;
    while (1){
        prev2 = prev1;
        prev1 = curr;
        if (cellNeighbor(maze, dim, curr, prev2, 'R') == 0){
//            printf("/n  river disappeared at %d\n", curr);
            maze[11*curr] = 'D';
            for (int i = 1; i <= 4; ++i){
                if(((maze[11*curr + i] == -1) || (maze[11*curr + i] == 0)) && (cellNeighbor(maze, dim, curr, prev2, 'R') == 0)){
                    maze[11*prev2 + 5] = curr;
                    maze[11*prev1 + 5] = curr;
                    return river_len;
                } 
            }
            return 0;
        }
        curr = maze[11*curr + cellNeighbor(maze, dim, curr, prev2, 'R')]; //find next river cell
        maze[11*prev2 + 5] = curr;
        ++river_len;
        /*
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("                    ");
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("step to %d", curr);
        */
    }
}


//player:
//[, cell, lives, bullets, bombs, treasure, ]
/*
cell:
["description", up, right, down, left, on_enter, w_up, w_right, w_down, w_left, items]
maze example:
[ , 'R', 2, 6, 8, 4, 3, 0, 0, 0, 0, 0, 'R', 3, -1, 9, 5, 3, 0, 0, 0, 0, 0,  ]
     |                              |   |                               |
     +-----------cell № 5-----------+   +-----------cell № 6------------+

view:
+-+-+-+
|r| |h|
+ + + +
|r e|e 
+ + + +
|d| |h|
+-+-+-+ 
*/

int playerWalk(int* maze, const int dim, int* players, const int players_N,  const int player){
    int curr_cell = players[5*player];
    int next_cell = curr_cell;
    //action control by letter
    //s -- shoot
    //b -- bomb
    //d -- drop
    //TODO: disable 'p'
    //p -- print maze as table for debug purposes
    int act = getchar();
    while(act != 27 && act != 's' && act != 'b' && act != 'd' && act != 'p'){
          act = getchar();
      printf("%d\n", act);
    }  
    //move:
//    printf("move player\n");    
    if (act == 27){
        //direcion control by arrows
        //ask user for command
        int a[4] = {1, 3, 2, 4};
        
        int dir = 0;
        if(getchar() == 91){
            dir = getchar() - 65;
        }
        dir = a[dir];
        printf("%d\n", dir);
        
        next_cell = maze[11*curr_cell + dir];
  if (maze[11*curr_cell + 5 + dir] == 0){
          //apply command (walk to )
          if (maze[11*next_cell] == 'R'){
            if ((maze[11*curr_cell] != 'R') && (maze[11*curr_cell] != 'D')){
              next_cell = maze[11*next_cell + 5];
            }
              
          }
          if (maze[11*next_cell] == '0'){
            next_cell = maze[11*next_cell + 5];
          }






       //    printf("go to %d\n", next_cell);
    if (next_cell != -1){
            players[5*player] = next_cell;
          }
  }
    }else{
// print maze as table. Useful for debug  
  if(act == 'p'){
    for (int cell = 0; cell <= dim*dim; ++cell){
      for(int i = 0; i < 11; ++i){
        printf(" %3d ", maze[cell*11 + i]);
      }
      printf("\n");
    }
      }
        if(act == 'd'){
            int tmp = players[5*player + 4];
            players[5*player + 4] = maze[11*curr_cell + 10];
            maze[11*curr_cell + 10] = tmp;
        }else{
    //input direction to shoot/bomb
            while(getchar() != 27){
            }
            int a[4] = {1, 3, 2, 4};
            int dir = 0;
            if(getchar() == 91){
                dir = getchar() - 65;
            }
            dir = a[dir];
      //bombing
            if(act == 'b'){
              if (players[5*player + 3] > 0){
//              if (1){
                --players[5*player + 3];
                if (maze[11*curr_cell + dir] != -1){
                  maze[11*curr_cell + 5 + dir] = 0;
                  int contr_dir = dir + 2;
                  if (contr_dir > 4){
                    contr_dir -= 4;
                }
                  maze[11*maze[11*curr_cell + dir] + 5 + contr_dir] = 0;
                }
              }
            }
      //shooting
            if(act == 's'){
              if (players[5*player + 2] > 0){
                --players[5*player + 2];
                printf("shooting at %d \n", dir);
                shoot(maze, dim, players, players_N, curr_cell, dir);
              }
            }
        }
    }
    
    //is it armory?
    if (maze[11*next_cell] == 'W'){
        players[5*player + 3] = 3;
        players[5*player + 2] = 3;
        
    }
    //is it hospital?
    if (maze[11*next_cell] == 'H'){
        players[5*player + 1] = 3;
    }
    //is it exit?
    if (next_cell == 0){
        if(players[5*player + 4] == 1){
    printf("player %d win\n", player);
  }
  if(players[5*player + 4] > 1){
    printf("player %d exited from maze with fake treasure\ndo you want to go back? [Y/n]", player);
    char go_back = getchar();
    printf("\n");
    if (go_back == 'Y' || go_back == 'y'){
      printf("do you want to drop your fake treasure outside?[Y/n] ");
      char drop_treasure = getchar();
      printf("\n");
      if(drop_treasure == 'Y' || drop_treasure == 'y'){
        players[5*player + 4] = 0;
      }
      players[5*player] = curr_cell;
    }else{
      printf("You lose.\n");
    }
  }



    }


    printPlayer(maze, dim, players, player, -1);
    
    return 0;    
}


int printPlayer(const int *maze, const int dim, int *players, const int player, const int viewed_R){
    printf("lives:   %d/3 \nbullets: %d/3 \nbombs:   %d/3\ncell:    %d\n", players[5*player + 1], players[5*player + 2], players[5*player + 3], players[5*player]);
    printf("cell data: %c ", maze[11*players[5*player]]);
    for (int i = 1; i <= 10; ++i){
      printf("  %3d", maze[11*players[5*player]+ i]);
    }
    printf("\n");
    printf("player data: ");
    for (int i = 0; i < 6; ++i){
        printf("%d ", players[5*player + i]);
    }
    printf("\n");
    int* maze1 = (int *)malloc(dim*(dim+1)*11 * sizeof(int));
    for(int i = 0; i < dim*(dim+1)*11; ++i){
        maze1[i] = maze[i];
    }
    maze1[11*players[5*player]] = 'P';
    MazePrint(maze1, dim);
    free(maze1);
    return 1;
    
}


int PlaceWall(int *maze, const int dim, const int cell_a, const int dir_a){
  int cell_b = maze[cell_a*11 + dir_a];
  int biome_a = maze[cell_a*11];
  int biome_b = maze[cell_b*11];
  if (biome_a == 'D'){
    biome_a = 'R';
  }
  if (biome_b == 'D'){
    biome_a = 'R';
  }

  if((0 < cell_b) && (cell_b < dim*dim) && (! ((biome_a == 'R') && (biome_b == 'R')))){
    maze[cell_a*11 + 5 + dir_a] = 1;
    int dir_b = dir_a + 2;
    if (dir_b > 4){
      dir_b -= 4;
    }
    maze[cell_b*11 + 5 + dir_b] = 1;
    return 0;
  }else{
    return 1;
  }

}




int shoot(const int *maze, int dim, int *players, int players_N, int curr_cell, int dir){
  //bullet goes throw the walls
  int next_cell = maze[11*curr_cell + dir];
  int players_damaged;
  while (next_cell != -1){
//    printf("next_cell == %d\n", next_cell);
    for (int i = 0; i < players_N; ++i){
      if (players[5*i] == next_cell){
        if (players[5*i + 1] > 0){
          players[5*i + 1] -= 1;
          ++players_damaged;
        }
      }
    }
    next_cell = maze[11*next_cell + dir];
  }
  return players_damaged;
}


/*
//width-first search
int CellsAvailable(int* maze, const int dim, const int start){
  const int total_cells = dim*dim + 1
  int *cells_visited = (int *)malloc(total_cells * sizeof(int));
  int *queue = (int *)malloc(total_cells * sizeof(int));
  for (int i = 0; i < total_cells; ++i){
    cells_visited[i] = -1;
    queue[i] = -1;
  }
  int cells_available[5];
  int cells_unvisited = total_cells;
  int max_iterations = dim*dim*dim;
  while (cells_unvisited && --max_iterations){
    int j = -1;
    for (int i = 0; i < 5; ++i){
      cells_avilable[i] = -1;
      next_cell = maze[11*curr_cell + i]
      if ((((maze[11*curr_cell + i] != 'R') && (maze[11*curr_cell + i] != 'D')) && ((maze[11*next_cell + i] == 'D') || (maze[11*next_cell + i] == 'R'))) && (maze[11*curr_cell + i] == '0')){
        cell_available = maze[11*next_cell + i];
      }else{
        cell_available = maze[11*curr_cell + i];
      }
      if(! CellVisited(cells_visited, curr_cell)){
        cells_available[++j] = cell_available;
      


      }
    }
    AddCells(cells_visited, queue, total_cells, cells_available);
    curr_cell = FirstCell(queue, total_cells); //pop first item
  }

  int curr_cell = start;



}
*/

