/********************************
 * ball.c
 * 
 * Author: Adam Miyashiro
 * Created: 4/21/16
 * Modified: 4/21/16
 * Part of: ball.h
 * Assignment: Final
 */

#include "ball.h"
#include "floor.h"
#include <glcdfont.h>
#include <f3d_lcd_sd.h>
#include <stdlib.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <stdio.h>
#include <stm32f30x.h>
#include <math.h>


// set up initial ball values
// return pointer to a ball
Ball_t *init_ball(void) {
  Ball_t *ball = (Ball_t *)malloc(sizeof(Ball_t));
  if (ball == NULL) {
    printf("Bad malloc");
    exit(0);
  }
  
  ball->x_pos = 50;
  ball->y_pos = 50;
  ball->radius = 7;
  ball->bottom = ball->y_pos + ball->radius;
  ball->top = ball->y_pos - ball->radius;
  ball->left_edge = ball->x_pos - ball->radius;
  ball->right_edge = ball->x_pos + ball->radius;
  ball->vert_speed = 0;
  ball->lat_speed = 0;

  return ball;
}


// draw given ball to the screen
void draw_ball(Ball_t *ball) {
  uint8_t y, z;
  uint16_t y0, y1;
  uint16_t x[13];
  double distance;

  y0 = ball->top;
  y1 = y0 + 1;
  
  for (z = 0; z < 13; z++) { 
    for (y = 0; y < 13; y++) {
      distance = dist((double)y, (double)z, (double)6, (double)6);
      
      if (distance <= (double)6.5) {
	x[y] = RED;
      }
      else {
	x[y] = WHITE;
      }
    }
    
    f3d_lcd_setAddrWindow (ball->left_edge, y0, ball->right_edge, y1, MADCTLGRAPHICS);
    f3d_lcd_pushColor(x, 13);

    y0++, y1++;
  }  
}


// clear given ball from the screen
void clear_ball(Ball_t *ball) {
  uint8_t y, z;
  uint16_t x[13];
  
  for (z = 0; z < 13; z++) { 
    for (y = 0; y < 13; y++) {
      x[y] = WHITE;
    }

    f3d_lcd_setAddrWindow (ball->left_edge, ball->top + z, ball->right_edge, (ball->top + 1) + z, MADCTLGRAPHICS);
    f3d_lcd_pushColor(x, 13);
  }  
}


// calculate distance between two points
double dist(double x1, double y1, double x2, double y2) {
  double d;
  double xd;
  double yd;

  xd = pow( (x2 - x1) , 2 );
  yd = pow( (y2 - y1) , 2 );

  d = sqrt( xd + yd );
  return d;
} 


// move the ball according to its current speed
void move_ball(Ball_t *ball, Node_t *queue) {
  //delay(5);
  clear_ball(ball);

  // update vertical ball speed if needed
  Floor_t *next_floor;
  next_floor = get_next_floor(ball, queue);
 
  if (next_floor == NULL) {
    ball->vert_speed += 1;
  }
  else if ((ball->bottom < next_floor->bottom &&
	    ball->bottom > next_floor->top) && 
	   (ball->left_edge < next_floor->gap_start ||
	    ball->right_edge > next_floor->gap_end)) {
    ball->vert_speed = 0;
    ball->y_pos = next_floor->top - ball->radius - 2;
  }
  else if ((ball->bottom < (ST7735_height - 1) ||
	    next_floor == NULL) && 
	   (ball->bottom < next_floor->top ||
	    (ball->left_edge >= next_floor->gap_start &&
	     ball->right_edge <= next_floor->gap_end)) &&
	   ball->vert_speed + 1 < 5) {
    ball->vert_speed += 1;
  }
  
  if (next_floor == NULL) {
    if (ball->bottom + ball->vert_speed < ST7735_height - 1) {
      ball->y_pos = ball->y_pos + ball->vert_speed;
    }
    else {
      ball->y_pos = (ST7735_height - 1) - ball->radius;
      ball->vert_speed = 0;
    }
  }
  else if (ball->bottom + ball->vert_speed < next_floor->top ||
	   (ball->left_edge >= next_floor->gap_start &&
	    ball->right_edge <= next_floor->gap_end)) {
    ball->y_pos = ball->y_pos + ball->vert_speed;
  }
  else {
    if (next_floor == NULL) {
      ball->y_pos = (ST7735_height - 1) - ball->radius;
      ball->vert_speed = 0;
    }
    else {
      ball->y_pos = next_floor->top - ball->radius;
      ball->vert_speed = 0;
    }
  }

  // update lateral ball speed if needed
  int roll_deg;
  roll_deg = (int)(get_board_roll() / 5);

  if (roll_deg == 0) {
    ball->lat_speed = 0;
  }
  else if (roll_deg < 0) {
    roll_deg = abs(roll_deg);
    if (ball->right_edge < (ST7735_width - 1) && ball->lat_speed  < 6) {
      ball->lat_speed += (int)roll_deg;
    }
    else {
      ball->lat_speed = 0;
    }
  }
  else {
    roll_deg = abs(roll_deg);
    if (ball->left_edge > 0 && ball->lat_speed > -6) {
      ball->lat_speed -= (int)roll_deg;
    }
    else {
      ball->lat_speed = 0;
    }
  }

  
  if (ball->lat_speed > 0) {
    if (ball->right_edge + ball->lat_speed < ST7735_width - 1) {
      ball->x_pos = ball->x_pos + ball->lat_speed;
    }
    else {
      ball->x_pos = (ST7735_width - 1) - ball->radius;
      ball->lat_speed = 0;
    }
  }
  else if (ball->lat_speed < 0) {
    if (ball->left_edge + ball->lat_speed > 0) {
      ball->x_pos = ball->x_pos + ball->lat_speed;
    }
    else {
      ball->x_pos = 0 + ball->radius;
      ball->lat_speed = 0;
    }
  }
  

  update_ball(ball);
  draw_ball(ball); 
  //delay(5);
}


// update ball attributes
void update_ball(Ball_t *ball) {
  ball->bottom = ball->y_pos + ball->radius;
  ball->top = ball->y_pos - ball->radius;
  ball->left_edge = ball->x_pos - ball->radius;
  ball->right_edge = ball->x_pos + ball->radius;
}


double get_board_roll(void) {
  float accel_data[3];
  double accel_pitch, accel_pitch_deg;
  double accel_roll, accel_roll_deg;
  
  f3d_accel_read(accel_data);

  accel_roll = atan2( accel_data[1], sqrt(pow(accel_data[0], 2) + pow(accel_data[2], 2)));
  accel_roll_deg = (180/M_PI) * accel_roll;
       
  return accel_roll_deg;
}


// check for ball position for game over
// return 1 if the game is over, 0 otherwise
int game_over(Ball_t *ball) {
  if (ball->top <= 0) {
    return 1;
  }
  else {
    return 0;
  }
}
