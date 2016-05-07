/********************************
 * floor.h
 * 
 * Author: Adam Miyashiro
 * Created: 4/21/16
 * Modified: 4/21/16
 * Part of: floor.c
 * Assignment: Final
 */

#ifndef floor_h_
#define floor_h_ 

#include "ball.h"

typedef struct Ball Ball_t;

typedef struct Floor {
  int top;
  int bottom;
  int gap_start;
  int gap_end;

  struct Floor *next;
} Floor_t;

typedef struct Node {
  Floor_t *floor;
  struct Node *next;
} Node_t;

Node_t *new_node();
void drawFloor(Floor_t *);
void clearFloor(Floor_t *);
Node_t *pop_floor(Node_t *);
void push_floor(Node_t *);
Floor_t *make_new_floor(void);
Node_t *move_floors_up(Node_t *, int, int *);
Floor_t *get_next_floor(Ball_t *, Node_t *);

#endif
