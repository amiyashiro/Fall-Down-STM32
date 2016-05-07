/*****************************************
 * main.c
 *
 * Author: Adam Miyashiro
 * Created: 4/20/16
 * Modified: 4/20/16
 * Part of: Final
 * Assignment: Final
 */

#include <stm32f30x.h> // Pull in include files for F30x standard drivers
#include <f3d_uart.h>
#include <f3d_lcd_sd.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <stdio.h>
#include <math.h>
#include <f3d_rtc.h>
#include <f3d_timer2.h>
#include <f3d_dac.h>
#include <f3d_dac.h>
#include <ff.h>
#include <diskio.h>
#include <stdlib.h>
#include <string.h>
#include <f3d_delay.h>
#include <stm32f30x_misc.h>
#include "ball.h"
#include "floor.h"


#define TIMER 20000
#define AUDIOBUFSIZE 128

extern uint8_t Audiobuf[AUDIOBUFSIZE];
extern int audioplayerHalf;
extern int audioplayerWhole;

FATFS Fatfs;        /* File system object */
FIL fid;        /* File object */
BYTE Buff[512];        /* File read buffer */
int ret;

struct ckhd {
  uint32_t ckID;
  uint32_t cksize;
};

struct fmtck {
  uint16_t wFormatTag;      
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
};



// all initializations
void init() {

  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
 
  f3d_uart_init();
  f3d_lcd_init();
  f3d_user_btn_init();
  f3d_timer2_init();
  f3d_dac_init();
  f3d_delay_init();
  f3d_rtc_init();
  f3d_i2c1_init();
  delay(10);
  f3d_accel_init();
  delay(10);

  f3d_lcd_fillScreen2(WHITE);
  f3d_lcd_fillScreen2(BLUE);
  f3d_lcd_fillScreen2(WHITE);
  f3d_lcd_fillScreen2(BLUE);
  f3d_lcd_fillScreen2(WHITE);
 
}


void readckhd(FIL *fid, struct ckhd *hd, uint32_t ckID) {
  f_read(fid, hd, sizeof(struct ckhd), &ret);
  if (ret != sizeof(struct ckhd))
    exit(-1);
  if (ckID && (ckID != hd->ckID))
    exit(-1);
}

void die (FRESULT rc) {
  printf("Failed with rc=%u.\n", rc);
  while (1);
}


// plays a .wav file from the SD card
int play(char* this){
  FRESULT rc;			/* Result code */
  DIR dir;			/* Directory object */
  FILINFO fno;			/* File information object */
  UINT bw, br;
  unsigned int retval;
  int bytesread;

  printf("Reset\n");
    
  f_mount(0, &Fatfs);/* Register volume work area */

  printf("\nOpen %s\n", this);
  rc = f_open(&fid, this, FA_READ);
  
  if (!rc) {
    struct ckhd hd;
    uint32_t  waveid;
    struct fmtck fck;
    
    readckhd(&fid, &hd, 'FFIR');
    
    f_read(&fid, &waveid, sizeof(waveid), &ret);
    if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
      return -1;
    
    readckhd(&fid, &hd, ' tmf');
    
    f_read(&fid, &fck, sizeof(fck), &ret);
    
    // skip over extra info
    
    if (hd.cksize != 16) {
      printf("extra header info %d\n", hd.cksize - 16);
      f_lseek(&fid, hd.cksize - 16);
    }
    
    printf("audio format 0x%x\n", fck.wFormatTag);
    printf("channels %d\n", fck.nChannels);
    printf("sample rate %d\n", fck.nSamplesPerSec);
    printf("data rate %d\n", fck.nAvgBytesPerSec);
    printf("block alignment %d\n", fck.nBlockAlign);
    printf("bits per sample %d\n", fck.wBitsPerSample);
    
    // now skip all non-data chunks !
    
    while(1){
      readckhd(&fid, &hd, 0);
      if (hd.ckID == 'atad')
	break;
      f_lseek(&fid, hd.cksize);
    }
    
    printf("Samples %d\n", hd.cksize);
    
    // Play it !
    
    //      audioplayerInit(fck.nSamplesPerSec);
    
    f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
    hd.cksize -= ret;
    audioplayerStart();
    while (hd.cksize) {
      int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
      if (audioplayerHalf) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(Audiobuf, AUDIOBUFSIZE/2);
	f_read(&fid, Audiobuf, next, &ret);
	hd.cksize -= ret;
	audioplayerHalf = 0;
      }
      if (audioplayerWhole) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	hd.cksize -= ret;
	audioplayerWhole = 0;
      }
    }
    audioplayerStop();
  }
  
  printf("\nClose the file.\n"); 
  rc = f_close(&fid);

  if (rc) die(rc);
}


void main() {
  init();
  char message[20];
  char message2[20];
  char end[10];
  int score = 0;
  int old_score = 0;
  int speed = 10;          // value for delay
  int pixel = 1;           // number of pixels to move floors
  char display_score[20];
  snprintf(message, 20, "Press the user");
  snprintf(end, 20, "GAME OVER");
  

  while (1) {
   
    snprintf(message2, 20, "button to start");
    f3d_lcd_drawString(20, 70, message, BLUE, WHITE);
    f3d_lcd_drawString(20, 80, message2, BLUE, WHITE);
       
    while (user_btn_read() == 0) {}

    f3d_lcd_fillScreen(WHITE);

    score = 0;
    snprintf(display_score, 20, "Score: %d", score);
    f3d_lcd_drawString(0, 0, display_score, BLACK, WHITE);

    speed = 10;
    pixel = 1;

    Node_t *queue = NULL; 
    queue = new_node();
    Ball_t *ball = NULL;
    ball = init_ball();

    push_floor(queue);
    draw_ball(ball);
   
    while(1) {
    
      queue = move_floors_up(queue, pixel, &score);
      move_ball(ball, queue);
      delay(speed);

      if (score != old_score) {
	snprintf(display_score, 20, "Score: %d", score);
	f3d_lcd_drawString(0, 0, display_score, BLACK, WHITE);
	old_score = score;

	if (score == 10) {
	  speed = 5;
	}
	else if (score == 20) {
	  speed = 3;
	}
	else if (score == 30) {
	  speed = 0;
	}
	else if (score == 40) {
	  speed = 2;
	  pixel = 2;
	}
	else if (score == 50) {
	  speed = 0;
	}
      }
    
      if (game_over(ball)) {
	break;
      }
    }
    
    
    f3d_lcd_drawString(37, 60, end, RED, WHITE);
    f3d_lcd_drawString(37, 70, display_score, RED, WHITE);

    snprintf(message2, 20, "button to reset");
    f3d_lcd_drawString(20, 100, message, BLUE, WHITE);
    f3d_lcd_drawString(20, 110, message2, BLUE, WHITE);
    
    play("boo.wav");
    
    while (user_btn_read() == 0) {}

    f3d_lcd_fillScreen(WHITE);
  }
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
/* Infinite loop */
/* Use GDB to find out why we're here */
  while (1);
}
#endif
