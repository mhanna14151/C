#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define MAZE_DIM 10
#define FALSE 0
#define TRUE 1
#define GRID_SIZE 10
#define MAX_INPUT_SIZE 100

/*

I referenced the code from the following sources to create this program:
- Assignment 3 Solutions
- BFS Solutions from the latest homework
- The Random Dice code you provided us

*/

// Provides the seed for random to base it's work off of
void randomseed() {
 	time_t t;   /* Built-in type for time in stdlib */
	srand((unsigned) time(&t));
}

// Returns a roll
int randomroll() {
	int sides = 12;
	int roll = (rand() % sides);
	return roll;
}

int waterwalk = 0; // If waterwalk == 1, you are able to walk on water

// Checks if the location given is a accessible location. 
// Returns 0 if False, 1 if True
// walls[row][col] = 1, denotes a wall at that location.
// walls[row][col] = 2, denotes a water tile at that location.
// waterwalk = 1, states you have the ability to walk on water.
int valid_place(int row, int col, int walls[GRID_SIZE][GRID_SIZE]) {
	if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
		return 0;
	}
	if (walls[row][col] == 1) {
		return 0;
	}
	if ((walls[row][col] == 2) &&  (waterwalk == 1)) {
		return 1;
	}
	if ((walls[row][col] == 2) && (waterwalk == 0)) {
		return 0;
	}
	else {
		return 1;
	}
}

int wall[MAZE_DIM][MAZE_DIM];
char predecessor[MAZE_DIM][MAZE_DIM];
struct location {
    int r;
    int c;
};
struct node {
    struct location loc;
    struct node *next;
};
struct queue {
    struct node *head;
    struct node *last;
};
struct location dequeue(struct queue *q) {
    if (q->head == NULL) {
        struct location not_found = { .r = -1, .c = -1};
        return not_found;
    }
    struct node *to_delete = q->head;
    q->head = q->head->next;
    struct location to_return = to_delete->loc;
    free(to_delete);
    if (q->head == NULL) {
        q->last = NULL;
    }
    return to_return;
}
struct queue *enqueue(struct location loc, struct queue *q) {
    struct node* new_node = malloc(sizeof(struct node));
    new_node->loc = loc;
    new_node->next = NULL;
    if (q->head == NULL) {
        q->head = new_node;
        q->last = new_node;
    } else {
        q->last->next = new_node;
    }
    q->last = new_node;
    return q;
}
struct queue *new_queue() {
    struct queue *q = malloc(sizeof(struct queue));
    q->head = NULL;
    q->last = NULL;
    return q;
}

void print_queue(struct queue *q) {
    for (struct node *n = q->head; n != NULL; n = n->next) {
        printf("(%d,%d) ", n->loc.r, n->loc.c);
    }
    printf("\n");
}

int is_legal(struct location loc, int wall[MAZE_DIM][MAZE_DIM],
             char prev[MAZE_DIM][MAZE_DIM]);

/* For debugging */
/*
void print_map(int wall[MAZE_DIM][MAZE_DIM]) {
    for (int r = 0; r < MAZE_DIM; r++) {
        for (int c = 0; c < MAZE_DIM; c++) {
            if (wall[r][c]) {
                printf("X");
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}
*/

// Get a list of locations to explore adjacent to this one
struct queue *get_neighbors(int r, int c, int wall[MAZE_DIM][MAZE_DIM],
        char prev[MAZE_DIM][MAZE_DIM]) {
    struct queue *neighbors = new_queue();
    struct location north;
    north.r = r-1;
    north.c = c;
    struct location east;
    east.r = r;
    east.c = c+1;
    struct location south;
    south.r = r+1;
    south.c = c;
    struct location west;
    west.r = r;
    west.c = c-1;
    if (is_legal(north, wall, prev)) {
        neighbors = enqueue(north, neighbors);
        prev[north.r][north.c] = 'S';
    }
    if (is_legal(east, wall, prev)) {
        neighbors = enqueue(east, neighbors);
        prev[east.r][east.c] = 'W';
    }
    if (is_legal(south,wall, prev)) {
        neighbors = enqueue(south, neighbors);
        prev[south.r][south.c] = 'N';
    }
    if (is_legal(west,wall, prev)) {
        neighbors = enqueue(west, neighbors);
        prev[west.r][west.c] = 'E';
    }
    return neighbors;
}

int is_legal(struct location loc, int wall[MAZE_DIM][MAZE_DIM],
             char prev[MAZE_DIM][MAZE_DIM]) {
    return loc.r >= 0 && loc.r < GRID_SIZE && loc.c >= 0 && loc.c < GRID_SIZE 
            && !wall[loc.r][loc.c] && !prev[loc.r][loc.c];
}

/* Merge queues:  good for adding neighbors to the end of the line.
 * The queue listed first goes first. */
struct queue *merge_queues(struct queue *q1, struct queue *q2) {
    if (q1->head == NULL) {
        q1->head = q2->head;
        q1->last = q2->last;
    } else {
        q1->last->next = q2->head;
    }
    if (q2->last != NULL) {
        q1->last = q2->last;
    }
    free(q2);  /* Just frees the wrapper */
    return q1;
}

int is_empty(struct queue *q) {
    return (q == NULL || q->head == NULL);
}
// Input solver was modified to be given a 2D array to solve.
// This will be used in the function make_game, which will eventually
// be utilized in a while loop in the main function.

int solver(char input_line[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (input_line[r][c] == 'X') {
                wall[r][c] = TRUE;
            } else {
                wall[r][c] = FALSE;
            }
        }
    }
    //printf("\n");

    /* Step 2:  Explore */
    struct queue *q = new_queue();
    struct location start;
    start.r = start.c = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            predecessor[i][j] = 0;  /* "Unexplored" */
        }
    }
    predecessor[0][0] = '*';  /* Special character to start */ 
    q = enqueue(start,q); 
    while (!is_empty(q)) {
        struct location to_explore = dequeue(q);
        if (to_explore.r == GRID_SIZE-1 && to_explore.c == GRID_SIZE-1) {
            /* Found it!  Print the result and quit */
            
        //I commented out the below, but left it for testing purposes,
        //especially if you wanted to confirm my bfs 
     /*  char solution[GRID_SIZE][GRID_SIZE];
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    if (wall[i][j]) {
                        solution[i][j] = 'X';
                    } else {
                        solution[i][j] = '-';
                    }
           		 }
            }
            
           	int sol_r = GRID_SIZE-1;
            int sol_c = GRID_SIZE-1;
            while (sol_r != 0 || sol_c != 0) {
                solution[sol_r][sol_c] = '*';
                if (predecessor[sol_r][sol_c] == 'N') {
                    sol_r--;
                } else if (predecessor[sol_r][sol_c] == 'E') {
                    sol_c++;
                } else if (predecessor[sol_r][sol_c] == 'S') {
                    sol_r++;
                } else if (predecessor[sol_r][sol_c] == 'W') {
                    sol_c--;
                } 
            }
            solution[0][0] = '*';
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    printf("%c", solution[i][j]);
                }
                printf("\n");
            }*/
            //printf("Path found!\n");
            return 0;
        }
        struct queue *neighbors = get_neighbors(to_explore.r, to_explore.c, wall, predecessor);
        q = merge_queues(q, neighbors);
    }
    return 1;
}
int irow;
int icol;
int item_row() {
	irow = (rand() % (GRID_SIZE-1));
	return irow;
}
int item_col() {
	icol = (rand() % (GRID_SIZE-1)) + 1;
	return icol;
}
//solver2 is to find the 'I' for water
int solver2(char input_line[GRID_SIZE][GRID_SIZE]) {
    /* Step 1:  Read the input */
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if ((input_line[r][c] == 'X') || (input_line[r][c] == 'W')) {
                wall[r][c] = TRUE;
            } else {
                wall[r][c] = FALSE;
            }
        }
    }
    //printf("\n");

    /* Step 2:  Explore */
    struct queue *q = new_queue();
    struct location start;
    start.r = start.c = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            predecessor[i][j] = 0;  /* "Unexplored" */
        }
    }
    predecessor[0][0] = '*';  /* Special character to start */ 
    q = enqueue(start,q); 
    while (!is_empty(q)) {
        struct location to_explore = dequeue(q);
        if (to_explore.r == irow && to_explore.c == icol) {
            //printf("Path found!\n");
            return 0;
        }
        struct queue *neighbors = get_neighbors(to_explore.r, to_explore.c, wall, predecessor);
        q = merge_queues(q, neighbors);
    }
    return 1;
}


void print_board(int my_row, int my_col,
				int walls[GRID_SIZE][GRID_SIZE]) {
	for (int r = 0; r < GRID_SIZE; r++) {
		for (int c = 0; c < GRID_SIZE; c++) {
			if (my_row == r && my_col == c) {
				printf("@");
			} else if (walls[r][c] == 1) {
				printf("X");
			}else if (walls[r][c] == 2) {
				printf("W");
			}else if (walls[r][c] == 3) {
				printf("I");	 		
			} else {
			printf("-");
			}
		}
	printf("\n");
	}
}

int board_check(int my_row, int my_col,
				int walls[GRID_SIZE][GRID_SIZE],char board[GRID_SIZE][GRID_SIZE]) {
	for (int r = 0; r < GRID_SIZE; r++) {
		for (int c = 0; c < GRID_SIZE; c++) {
			if (my_row == r && my_col == c) {
				board[r][c]= '-';
			} else if (walls[r][c] == 0) {
				board[r][c]= '-';
			}else if (walls[r][c] == 1) {
				board[r][c]= 'X';
			} else {
				board[r][c] = 'W';
			}
		}
	}
	return 0;
}
int walls[GRID_SIZE][GRID_SIZE];
int my_row = 0;
int my_col = 0;

int makegame() {
	for (int r = 0; r < GRID_SIZE; r++) {
		for (int c = 0; c < GRID_SIZE; c++) {
			walls[r][c] = 0;
		}
	}
	char input_line[GRID_SIZE * (GRID_SIZE+2)];
	for (int r = 0; r < GRID_SIZE; r++) {
		for (int c = 0; c < GRID_SIZE; c++) {
				int a = randomroll();
				// printf ("****** %d \n", a);
				// shows the numbers for testing purposes
				if (a < 6) {
					walls[r][c] = 0;
				}
				else if ((a < 9) && (a > 5)) {
					walls[r][c] = 2;
				}
				else {
					walls[r][c] = 1;
				}
			}
		}
   	char inputtest[GRID_SIZE][GRID_SIZE]; 
	board_check(0, 0, walls, inputtest);
	// A good way to test if both my BFS searches are working
	// and that new mazes are not being created until both
	// conditions that a maze that both has an accessible
	// 'I' (namely the Item which grants the ability to waterwalk
	// or swim) and an accessible finish is to switch solver2
	// if (solver2(inputtest == 0). This will create mazes
	// until the exit is accessible ignoring water tiles, but
	// you'll be unable to complete it as the 'I', which
	// grants you the ability to pass through water tiles,
	// is no longer accessible.
	if (solver2(inputtest) == 1) {
		return 1;
	}
	if (solver(inputtest) == 1) {
		return 1;
	}
	else {
		return 0;
	}
}

int main() {
	randomseed();
	item_row();
	item_col();
	while (makegame() == 1) {
		makegame();
	}	
	printf("\nWelcome to the Game!\nYour goal is to reach the bottom right of the Maze!\n");
 	printf("If you reach the item ('I') you will learn the ability to traverse water tiles!\n");
 	printf("Happy exploring!\n");
 	printf("Choose a Direction! 'N', 'E', 'S', or 'W' to go in their respective cardinal directions:\n");
 	char itemlocation = walls[irow][icol] = 3;
 	char input[MAX_INPUT_SIZE];
 	while (my_row != GRID_SIZE-1 || my_col != GRID_SIZE-1) {
 		print_board(my_row, my_col, walls);
 		fgets(input, MAX_INPUT_SIZE, stdin);
 		if ((my_row == irow) && (my_col == icol)) {
 			waterwalk = 1;
 			walls[irow][icol] = 0;
 			printf("You have gained the ability to walk on water!\n");
 		}
 		int new_row = my_row;
 		int new_col = my_col;
 		if (input[0] == 'N') {
 			new_row--;
 		} else if (input[0] == 'E') {
 			new_col++;
 		} else if (input[0] == 'W') {
 			new_col--;
 		} else if (input[0] == 'S') {
 			new_row++;
 		} else if (input[0] == 'A') {
 			break;
 		}
 		if (valid_place(new_row, new_col, walls)) {
 			my_row = new_row;
 			my_col = new_col;
 		}
 		printf("Choose a Direction! 'N', 'E', 'S', or 'W' to go in their respective cardinal directions:\n");
 		if (waterwalk == 1) {
 			printf("You can traverse water tiles!\n");
 		}
 	}
 	print_board(my_row, my_col, walls);
 	printf("Congratulations! You have traversed the maze!\n");
 	return 0;
} 


