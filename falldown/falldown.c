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

	#define RED 255, 0, 0
	#define HEADER ttk_screen->wy
	#define SHEIGHT (ttk_screen->h - ttk_screen->wy)

	static PzModule *module;
	static TWindow *window;
	static TWidget *wid;
	static PzConfig *config;

	typedef struct line_struct {
		int y;
		int bx;
	} line_pos;

	typedef struct ball_struct {
		int x;
		int y;
	} ball_pos;

	static line_pos line;
	static ball_pos ball;
	static int timer[2], move;
	static int line_gone, mil;
	static int score, highscore, game_status;
	static ttk_surface ball_image;

	static void draw_ball (ttk_surface srf);
	static void draw_line (ttk_surface srf);
	static void new_line();
	static void game_over (ttk_surface srf);
	static void falldown_timer();
	static void reset_falldown();
	static int check_forline();


	static void draw_falldown (PzWidget *wid, ttk_surface srf)
	{
		 ttk_fillrect (srf, 0, 0, ttk_screen->w, ttk_screen->h, ttk_makecol(WHITE));
		 if((line_gone = check_forline()) == 1)
		 	  new_line();
		 draw_line(srf);
		 draw_ball(srf);
		 if(ball.y <= -15)
		 	 game_over(srf);
	   
	}

	static void draw_ball (ttk_surface srf)
	{
		 ttk_blit_image (ball_image, srf, ball.x, ball.y);
	   if((ball.y >= (line.y-17) && ball.y <= (line.y-5)) && ((ball.x <= (line.bx-5) || (ball.x >= (line.bx+20)))))
		 	 ball.y -= move;
		 else {
		 	 if(ball.y <= (SHEIGHT-18)) 
		 	    ball.y += 4;
		 }
		 line.y -= move;
	}

	static void draw_line (ttk_surface srf)
	{
		 ttk_fillrect (srf, 0, line.y, line.bx, line.y+8, ttk_makecol(BLACK));
		 ttk_fillrect (srf, line.bx+30, line.y, ttk_screen->w, line.y+8, ttk_makecol(BLACK));
	}

	static void new_line ()
	{
		 line.y = SHEIGHT;
		 line.bx = rand() % (ttk_screen->w-30);
	}

	static int check_forline()
	{
		 if(line.y <= -8)
		 	 return 1;
		 else
		 	 return 0;
	}

	static void game_over (ttk_surface srf)
	{
		 int width, height;
		 char str[10];
		 
		 if ( score > pz_get_int_setting(config, highscore)) { pz_set_int_setting(config, highscore, score); }
		 ttk_fillrect (srf, 0, 0, ttk_screen->w, ttk_screen->h, ttk_makecol(BLACK));
		 height = ttk_text_height (ttk_menufont);
		 pz_vector_string_center (srf, "Final Score", ttk_screen->w/2, 10, 8, 10, 1, ttk_makecol(RED));
		 sprintf(str, "%d", score);
		 pz_vector_string_center (srf, str, ttk_screen->w/2, 25, 8, 10, 1, ttk_makecol(RED));
		 
		 width = ttk_text_width (ttk_menufont, "High Score:");
		 ttk_text (srf, ttk_menufont, (ttk_screen->w/2)-(width/2), SHEIGHT/2, ttk_makecol(WHITE), "High Score:");
		 sprintf(str, "%d", pz_get_int_setting(config,highscore));
		 width = ttk_text_width (ttk_menufont, str);
		 ttk_text (srf, ttk_menufont, (ttk_screen->w/2)-(width/2), ((SHEIGHT/2)+height)+5, ttk_makecol(WHITE), str);
		 game_status = 1;
	}

	static void falldown_timer()
	{
		 timer[0]++;
	   timer[1]++;
	   if(mil < 20) {
	     if(timer[0]==20) {
	   	   mil--;
	   	   pz_widget_set_timer(wid, mil);
	   	   timer[0]=0;
	   	 }
	   }
	   if(timer[1]==200) {
	   	 move++;
	   	 timer[1]=0;
	   }
	   score += 3;
	   wid->dirty++;
	}

	static void reset_falldown()
	{
		 ball.x = (ttk_screen->w/2)-7;
		 ball.y = 0;
		 timer[0] = 0;
		 timer[1] = 0;
		 line.y = -8;
		 score = 0;
		 move = 2;
		 mil = 50;
		 game_status = 0;
	}

	static int event_falldown (PzEvent *e)
	{

	int ret = TTK_EV_CLICK;

	switch(e->type) {

	   case PZ_EVENT_SCROLL:
	     TTK_SCROLLMOD( e->arg, 1);
	      if( e->arg > 0){
	      	if(ball.x <= (ttk_screen->w-25))
	      		ball.x+=10;
	      }
	      else {
	      	if(ball.x >= 5)
	      		ball.x-=10;
	      }
	      break;

	   case PZ_EVENT_BUTTON_DOWN:
	      switch (e->arg) {
	      case PZ_BUTTON_ACTION:
	      	 if(game_status == 1) {
	      	 	 reset_falldown();
	      	 	 wid->dirty++;
	      	 }
	         break;

	      case PZ_BUTTON_MENU:
	      	 pz_save_config(config);
	      	 pz_close_window(e->wid->win);
	         break;
	         
	      default:
	         ret = TTK_EV_UNUSED;
	      }
	      break;

	   case PZ_EVENT_TIMER:
	   	  if(ball.y > -15)
	   	    falldown_timer();
	      break;

	   default:
	      ret = TTK_EV_UNUSED;
	      break;

	   }
	   return ret;

	}

	static PzWindow *new_falldown_window()
	{
		  reset_falldown();
		  srandom((unsigned)time(NULL));
		  ball_image = ttk_load_image(pz_module_get_datapath(module, "ball.gif"));
		
		  config = pz_load_config(pz_module_get_datapath(module,"falldown.conf"));
	    window = pz_new_window ("FallDown", PZ_WINDOW_NORMAL);
	    wid = pz_add_widget(window, draw_falldown, event_falldown);
	    pz_widget_set_timer(wid, mil);
	    return pz_finish_window (window);
	}


	static void init_falldown()
	{
	    module = pz_register_module ("falldown", NULL);
	    pz_menu_add_action ("/Extras/Games/Falldown", new_falldown_window);

	}

	PZ_MOD_INIT (init_falldown)
