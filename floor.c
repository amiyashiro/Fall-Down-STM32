/********************************
 * floor.c
 * 
 * Author: Adam Miyashiro
 * Created: 4/21/16
 * Modified: 4/21/16
 * Part of: floor.h
 * Assignment: Final
 */

#include "floor.h"
#include <glcdfont.h>
#include <f3d_lcd_sd.h>
#include <stm32f30x.h>
#include <stdlib.h>
#include <stdio.h>

#define COLOR BLACK


Node_t *new_node() {
  Node_t *new = (Node_t *)malloc(sizeof(Node_t));
  if (new == NULL) {
    printf("Bad malloc");
    exit(0);
  }
  
  new->floor = NULL;
  new->next = NULL;
  return new;  
}

void draw_floor(Floor_t *floor) {
  uint8_t y;
  uint16_t y0, y1;
  uint16_t x[ST7735_width];
  
  if (floor->top < 0) { y0 = 0; }
  else { y0 = floor->top; }
  if (floor->bottom > ST7735_height) { y1 = ST7735_height; }
  else { y1 = floor->bottom; }

  for (y = 0; y < ST7735_width; y++) {
    if (y >= floor->gap_start && y <= floor->gap_end) {
      x[y] = WHITE;
    }
    else {
      x[y] = COLOR;
    }
  }
  
  f3d_lcd_setAddrWindow (0, y0, ST7735_width-1, y1, MADCTLGRAPHICS);
  for (y = y0; y < y1; y++) {
    f3d_lcd_pushColor(x, ST7735_width);
  }
}


// draw white bar for the dimensions of given floor
void clear_floor(Floor_t *floor) {
  uint8_t y;
  uint16_t y0, y1;
  uint16_t x[ST7735_width];
  
  if (floor->top < 0) { y0 = 0; }
  else { y0 = floor->top; }
  if (floor->bottom > ST7735_height-1) { y1 = ST7735_height-1; }
  else { y1 = floor->bottom; }

  for (y = 0; y < ST7735_width; y++) x[y] = WHITE;
  f3d_lcd_setAddrWindow (0, y0, ST7735_width-1, y1, MADCTLGRAPHICS);
  for (y = y0; y < y1; y++) {
    f3d_lcd_pushColor(x, ST7735_width);
  }
}


// take off top floor of the queue
Node_t *pop_floor(Node_t *queue) {
  Node_t *front;
  front = queue->next;
  free(queue->floor);
  queue->floor = NULL;
  free(queue);
  queue = NULL;
  return front;
}


// add floor to the back of the queue
void push_floor(Node_t *queue) {
  Node_t *queue_ptr = queue;
  Floor_t *new_floor;
  new_floor = make_new_floor();

  if (queue->floor == NULL) {
    queue->floor = new_floor;
    return;
  }
  
  while(queue_ptr->next != NULL) {
    queue_ptr = queue_ptr->next;
  }
  
  Node_t *new = new_node();
  new->floor = new_floor;
  queue_ptr->next = new;
}

Floor_t *make_new_floor(void) {
  Floor_t *new_floor;
  new_floor = (Floor_t *)malloc(sizeof(Floor_t));
  if (new_floor == NULL) {
    printf("Bad malloc");
    exit(0);
  }
  
  new_floor->top = ST7735_height;
  new_floor->bottom = ST7735_height + 5;

  new_floor->gap_start = rand() % (ST7735_width - 34); // Randomly generate placement of hole
  new_floor->gap_end = new_floor->gap_start + 35;      // in each floor.

  return new_floor;
}


// move all floors up n pixels
// loops through floors, clearing, altering values,
// and drawing again
Node_t *move_floors_up(Node_t *queue, int n, int *score) {
  Node_t *queue_ptr = queue;

  clear_floor(queue_ptr->floor);
  queue_ptr->floor->top -= n;
  queue_ptr->floor->bottom -= n;

  if (queue_ptr->floor->bottom < 0) {
    queue = pop_floor(queue);
    queue_ptr = queue;
    *score += 1;
  }
  else {
    draw_floor(queue_ptr->floor);
  }
  
  while (queue_ptr->next != NULL) {
    queue_ptr = queue_ptr->next;
    clear_floor(queue_ptr->floor);
    queue_ptr->floor->top -= n;
    queue_ptr->floor->bottom -= n;
    draw_floor(queue_ptr->floor);
  }

  if (queue_ptr->floor->bottom < (ST7735_height - 25)) {
    push_floor(queue);
  }
  
  return queue;
}

// returns the floor in the queue that is closest to the bottom of the ball
Floor_t *get_next_floor(Ball_t *ball, Node_t *queue) {
  Node_t *queue_ptr = queue;

  while (queue_ptr != NULL) {
    if (queue_ptr->floor->bottom < ball->bottom) {
      queue_ptr = queue_ptr->next;
    } 
    else {
      break;
    }

    if (queue_ptr == NULL) {
      return NULL;
    }
  }

  return queue_ptr->floor;
}

