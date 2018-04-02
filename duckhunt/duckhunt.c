/*
 * Copyright (C) 2006 Armon Khosravi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "pz.h"
#include "duckhunt.h"

#define HEADER ttk_screen->wy
#define SWIDTH ttk_screen->w
#define SHEIGHT ttk_screen->h

#define MAX_BOUNCE 16

#define TOP 0
#define BOTTOM 116-30 //116
#define LEFT 0
#define RIGHT 220-28 //220

#define RED 255,0,0
#define YELLOW 255,253,58

static PzModule *module;
static TWindow *window;
static TWidget *wid;

static ttk_surface duckhunt_ss, duckhunt_bg, duckhunt_lose_bg, duck[3], 
       dogw[8], dead_duck, dd_fall, dog_duck, dog_laugh[2];
       
static int pause=0, ssFrame=0, action=0, start_duck=0, duck_timer=0;
static int ranbounce, duck_frame=0, dogw_frame=0, dogl_frame=0;
static int shots=3, score=0, dd_fally, duck_faway;
static int temp_bcx, temp_bcy;
static int aimerx, aimery;
static int duckx, ducky;
static int imgw, imgh;

static const int bounce_cords[MAX_BOUNCE][2] = {
	 {6, 4}, {-6, -4}, {3, 6}, {-3, -6},
	 {8, 5}, {-8, -5}, {2, 8}, {-2, -8},
	 {7, 3}, {-7, -3}, {-6, 4}, {-3, 6},
	 {10, 2}, {-10, -2}, {12, 5}, {-12, -5}
};
static int score_opts[3] = {500, 1000, 1500};


static void draw_duckhunt (PzWidget *wid, ttk_surface srf)
{
	 if (pause == 0)
	    startup_screen (srf);
	 else if (pause == 1) {
	 	  ttk_blit_image (duckhunt_bg, srf, 0, 0);
	 	  dog_walk(srf);
	 }
	 else if (pause == 2) {
	 	  duckhunt_score(srf);
	 	  if (duck_timer >= 100) ttk_blit_image (duckhunt_lose_bg, srf, 0, 0);
	 	  else ttk_blit_image (duckhunt_bg, srf, 0, 0);
      if (action == 1)
      	 aimer_hold (srf, aimerx, aimery);
      else if (action == 2)
      	 aimer_shoot (srf, aimerx, aimery);
      if (start_duck == 0)
         init_duck();
      else if (start_duck == 1) {
      	 if (duck_timer >= 100)
      	 	  duck_flyaway(srf);
      	 else
      	   move_duck(srf);
      }
      else if (start_duck == 2)
      	 duck_shot(srf);
      else if (start_duck == 3)
      	 dog_holdduck(srf);
      else if (start_duck == 4)
      	 dog_laughing(srf);
      draw_aimer (srf, aimerx, aimery);
   }
}

static void startup_screen (ttk_surface srf)
{
	 if (ssFrame == 0) {
	 	  ttk_fillrect (srf, 0, 0, 220, 176, ttk_makecol(BLACK));
	    ttk_blit_image (duckhunt_ss, srf, (220/2)-(imgw/2), ((176-HEADER)/2)-(imgh/2));
	    //ttk_text (srf, ttk_menufont, 0, 0, ttk_makecol(WHITE), "hello world");
	 }
	 else {
	 	  pause=1;
   }
}

static void duckhunt_score (ttk_surface srf)
{
	 char str[20];
	 
	 sprintf(str, "%d", score);
   pz_vector_string_center (srf, str, 220-5-21, 176-5-8, 5, 7, 1, ttk_makecol(WHITE));
}

static void duckhunt_message (ttk_surface srf, char *message)
{
	 int width, height;
	 
	 ttk_fillrect (srf, 0, 0, ttk_screen->w, ttk_screen->h, ttk_makecol(BLACK));

static void dog_walk (ttk_surface srf)
{
	 pz_widget_set_timer(wid, 700);
	 if (dogw_frame < 5) {
	    ttk_blit_image (dogw[dogw_frame], srf, dogw_frame*20, 103);
	    dogw_frame++;
	 }
	 else if (dogw_frame >= 5 && dogw_frame < 8) {
	 	  if (dogw_frame == 5)
	 	     ttk_blit_image (dogw[dogw_frame], srf, 80, 97);
	 	  else
	 	  	 ttk_blit_image (dogw[dogw_frame], srf, 80, 75);
	 	  dogw_frame++;
	 }
	 else if (dogw_frame == 8) {
	 	  pause=2;
	 	  draw_aimer (srf, aimerx, aimery);
	 	  pz_widget_set_timer(wid, 50);
	 	  wid->dirty++;
	 }  
}

static void dog_holdduck (ttk_surface srf)
{
	 pz_widget_set_timer(wid, 3000);
	 ttk_blit_image (dog_duck, srf, 110-22, 116-42);
	 start_duck=0;
	 shots=3;
}

static void dog_laughing (ttk_surface srf)
{
	 pz_widget_set_timer(wid, 300);
	 ttk_blit_image (dog_laugh[dogl_frame], srf, 110-15, 116-40);
	 dogl_frame++;
	 if (dogl_frame == 2) dogl_frame=0;
	 if (duck_timer >= 6) {
	 	  start_duck=0;
	 	  duck_timer=0;
	 	  shots=3;
	 }
}

static void duck_flyaway (ttk_surface srf)
{
	 pz_widget_set_timer(wid, 50);
	 ttk_blit_image (duck[duck_frame], srf, duckx, duck_faway);
	 duck_faway -= 5;
   duck_frame++;
	 if (duck_frame == 3) duck_frame=0;
	 if (duck_faway == -30) {
	 	  duck_timer=0;
	 	  start_duck=4; //DOG LAUGHS AT YOU
	 	  wid->dirty++;
	 }
}

static void duck_shot (ttk_surface srf)
{
	 pz_widget_set_timer(wid, 50);
	 dd_fally += 4;
	 ttk_blit_image (dd_fall, srf, duckx+5, dd_fally);
	 if (dd_fally >= (116-32)) {
	 	  start_duck=3;  //DOG HOLDS DEAD DUCK
	 }
}

static void move_duck (ttk_surface srf)
{
	 ttk_blit_image (duck[duck_frame], srf, duckx, ducky);
	 duckx += temp_bcx;
	 ducky += temp_bcy;
	 if (duckx <= LEFT)
	 	 	temp_bcx = temp_bcx + (temp_bcx*(-2));
	 else if (duckx >= RIGHT)
	 	 	temp_bcx = -temp_bcx;

	 if (ducky <= TOP)
	 	 	temp_bcy = temp_bcy + (temp_bcy*(-2));
	 else if (ducky >= BOTTOM)
	 	 	temp_bcy = -temp_bcy;
	 duck_frame++;
	 if (duck_frame == 3) duck_frame=0;
	 duck_faway = ducky;
}
	 

static void init_duck()
{
	 pz_widget_set_timer(wid, 50);
	 duckx = random() % 189;//221;
	 ducky = random() % 83;//177;
	 ranbounce = random() % MAX_BOUNCE;
	 temp_bcx = bounce_cords[ranbounce][0];
	 temp_bcy = bounce_cords[ranbounce][1];
	 start_duck = 1;
}
	 
static void draw_aimer (ttk_surface srf, int x, int y)
{
	 ttk_ellipse (srf, x, y, 5, 5, ttk_makecol(RED));
	 ttk_line (srf, x-5, y, x+5, y, ttk_makecol(RED));
	 ttk_line (srf, x, y-5, x, y+5, ttk_makecol(RED));
}

static void aimer_hold (ttk_surface srf, int x, int y)
{
	 ttk_fillellipse (srf, x, y, 5, 5, ttk_makecol(YELLOW));
}

static void aimer_shoot (ttk_surface srf, int x, int y)
{
	 ttk_fillellipse (srf, x, y, 5, 5, ttk_makecol(WHITE));
	 if ((aimerx > duckx && aimerx < duckx+28) && (aimery > ducky && aimery < ducky+30)) {
	 	  //DUCK IS HIT
	 	  pz_widget_set_timer(wid, 2000);
	 	  score += score_opts[shots-1];
	 	  start_duck=2;
	 	  dd_fally=ducky;
	 	  ttk_blit_image (duckhunt_bg, srf, 0, 0);
	 	  ttk_blit_image (dead_duck, srf, duckx, ducky);
	 }
	 if (shots > 0) shots--;
	 action=0;
}

static void move_up()
{
	 if(aimery >= 8) {
     aimery -= 5;
   }
}

static void move_down()
{
	 if(aimery <= ((SHEIGHT-HEADER)-7)) {
     aimery += 5;
   }
}

static void move_left()
{
	 if(aimerx >= 7) {
     aimerx -= 5;
   }
}

static void move_right()
{
	 if(aimerx <= (SWIDTH-7)) {
     aimerx += 5;
   }
}

static void hit_action()
{
   if (pause == 0)
      ssFrame++;
   wid->dirty++;
}

static void reset_duckhunt()
{
	 aimerx = SWIDTH/2;
	 aimery = (SHEIGHT-HEADER)/2;
	 ssFrame = 0;
	 action = 0;
	 pause = 0;
	 score = 0;
	 dogl_frame = 0;
	 duck_frame = 0;
	 duck_timer = 0;
	 dogw_frame = 0;
	 start_duck = 0;
	 shots = 3;
}

static int event_duckhunt (PzEvent *e)
{

int ret = TTK_EV_CLICK;

switch(e->type) {

   case PZ_EVENT_SCROLL:
     TTK_SCROLLMOD( e->arg, 10);
      if( e->arg > 0){break;}
      else {break;}
      
   case PZ_EVENT_BUTTON_DOWN:
      switch (e->arg) { 
      case PZ_BUTTON_ACTION:
      	 hit_action();
         break;
         
      case PZ_BUTTON_HOLD : 
      	 pz_close_window(e->wid->win);  
         break;
         
      default:
         ret = TTK_EV_UNUSED;
      }
      break;

   case PZ_EVENT_TIMER:
   	  if (pause == 2) {
   	     if (ttk_button_pressed (PZ_BUTTON_MENU)) move_up();
   	     if (ttk_button_pressed (PZ_BUTTON_PLAY)) move_down();
   	     if (ttk_button_pressed (PZ_BUTTON_PREVIOUS)) move_left();
   	     if (ttk_button_pressed (PZ_BUTTON_NEXT)) move_right();
   	     if (shots > 0) {
   	        if (ttk_button_pressed (PZ_BUTTON_ACTION)) action = 1;
   	     }
   	     if (start_duck == 1) duck_timer++;
   	     else if (start_duck == 4) duck_timer++;
      	 wid->dirty++;
   	  }
   	  else if (pause == 1) wid->dirty++;
      break;
      
   case PZ_EVENT_BUTTON_UP:
		 switch (e->arg) {
     case PZ_BUTTON_ACTION:
     	  if (shots > 0) {
     	     if (pause == 2 && action == 1) {
     	  	    action=2;
     	  	    wid->dirty++;
     	     }
     	  }
     default:
     	 break;
     }
     	  	 
   default:
      ret = TTK_EV_UNUSED;
      break;

   }
   return ret;

}

static PzWindow *new_duckhunt_window()
{
	  reset_duckhunt();
	  duckhunt_lose_bg = ttk_load_image (pz_module_get_datapath (module,"duckhunt_lose_bg.png"));
	  dog_laugh[0] = ttk_load_image (pz_module_get_datapath (module,"dog_laugh1.png"));
	  dog_laugh[1] = ttk_load_image (pz_module_get_datapath (module,"dog_laugh2.png"));
	  duckhunt_ss = ttk_load_image (pz_module_get_datapath (module,"duckhunt_ss.png"));
	  duckhunt_bg = ttk_load_image (pz_module_get_datapath (module,"duckhunt_bg.png"));
	  dead_duck = ttk_load_image (pz_module_get_datapath (module,"dead_duck.png"));
	  dog_duck = ttk_load_image (pz_module_get_datapath (module,"dog_duck.png"));
	  dd_fall = ttk_load_image (pz_module_get_datapath (module,"dd_fall.png"));
	  duck[0] = ttk_load_image (pz_module_get_datapath (module,"duck1.png"));
	  duck[1] = ttk_load_image (pz_module_get_datapath (module,"duck2.png"));
	  duck[2] = ttk_load_image (pz_module_get_datapath (module,"duck3.png"));
	  dogw[0] = ttk_load_image (pz_module_get_datapath (module,"dog1.png"));
	  dogw[1] = ttk_load_image (pz_module_get_datapath (module,"dog2.png"));
	  dogw[2] = ttk_load_image (pz_module_get_datapath (module,"dog3.png"));
	  dogw[3] = ttk_load_image (pz_module_get_datapath (module,"dog4.png"));
	  dogw[4] = ttk_load_image (pz_module_get_datapath (module,"dog1.png"));
	  dogw[5] = ttk_load_image (pz_module_get_datapath (module,"dog5.png"));
	  dogw[6] = ttk_load_image (pz_module_get_datapath (module,"dog6.png"));
	  dogw[7] = ttk_load_image (pz_module_get_datapath (module,"dog7.png"));
	  ttk_surface_get_dimen (duckhunt_ss, &imgw, &imgh);
	  srandom((unsigned)time(NULL));
	  
    window = pz_new_window ("DuckHunt", PZ_WINDOW_FULLSCREEN);
    wid = pz_add_widget(window, draw_duckhunt, event_duckhunt);
    pz_widget_set_timer(wid, 50);
    return pz_finish_window (window);
}

static void init_duckhunt()
{
    module = pz_register_module ("duckhunt", NULL);
    pz_menu_add_action ("/Extras/Games/DuckHunt", new_duckhunt_window);
}

PZ_MOD_INIT (init_duckhunt)
