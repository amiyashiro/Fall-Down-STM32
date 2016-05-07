/********************************
 * ball.h
 * 
 * Author: Adam Miyashiro
 * Created: 4/21/16
 * Modified: 4/21/16
 * Part of: ball.c
 * Assignment: Final
 */

#ifndef ball_h_
#define ball_h_

#include "floor.h"

typedef struct Node Node_t;

typedef struct Ball {
  int x_pos;
  int y_pos;
  int radius;
  int left_edge;
  int right_edge;
  int top;
  int bottom;
  int vert_speed;
  int lat_speed;

} Ball_t;


Ball_t *init_ball(void);
void draw_ball(Ball_t *);
void clear_ball(Ball_t *);
double dist(double, double, double, double);
void move_ball(Ball_t *, Node_t *);
void update_ball(Ball_t *);
void get_board_tilt(void);
double get_board_roll(void);
int game_over(Ball_t *);

#endif
