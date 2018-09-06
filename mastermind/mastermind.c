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
  #include <ctype.h>
  #include "pz.h"

  #define HEADER ttk_screen->wy

  #define RED 255, 0, 0
  #define BLUE 0, 0, 255
  #define YELLOW 255, 255, 0
  #define GREEN 57, 220, 0

  enum { INTRO, SELECTION, WON, LOSE } ;

  static PzModule *module;
  static TWindow *window;
  static TWidget *wid;

  static int w, h, bpp;
  static int menuframe;
  static int hidden[4];
  static int count = 6, paused = 0, selection = 1;
  static int s1=0,s2=1,s3=2,s4=3;

  ttk_color col1, col2, col3, col4;

  void reset_mastermind();
  void get_colours(ttk_surface srf, int c1, int c2, int c3, int c4);
  void ChangeSelection(int side);
  void ConfirmSelection();


  void draw_mastermind (PzWidget *wid, ttk_surface srf) {
     int i,j,p,t,guess[4];
     char str[10];
     
     ttk_fillrect (srf, 0, 0, w, h, ttk_makecol(BLACK));
     
     switch(menuframe)
     {
     	 case INTRO:
         for(i=0;i<4;i++)
  	       hidden[i] = random() % 4;
  	       
     	 	 pz_vector_string_center (srf, "Pick 4 colours out of", w/2, 5, 6, 6, 1, ttk_makecol(WHITE));
     	 	 pz_vector_string_center (srf, "red, blue,", w/2, 13, 6, 6, 1, ttk_makecol(WHITE));
         pz_vector_string_center (srf, "yellow, and green", w/2, 20, 6, 6, 1, ttk_makecol(WHITE));
     	 	 ttk_ellipse (srf, (w/5)-6, (h-HEADER)/2-3, 6, 6, ttk_makecol(WHITE));
     	 	 ttk_fillellipse (srf, (w/5)-6, (h-HEADER)/2-3, 5, 6, ttk_makecol(GREEN));
     	 	 pz_vector_string (srf, " = colour guessed \n in right position", (w/5), ((h-HEADER)/2)-6, 6, 6, 1, ttk_makecol(WHITE));
         ttk_ellipse (srf, (w/5)-6, (h-HEADER)/2+17, 6, 6, ttk_makecol(WHITE));
         ttk_fillellipse (srf, (w/5)-6, (h-HEADER)/2+17, 5, 6, ttk_makecol(BLUE));
         pz_vector_string (srf, " = colour guessed \n in wrong position", (w/5), ((h-HEADER))/2+14, 6, 6, 1, ttk_makecol(WHITE));
     	 	 break;
     	 	 
     	 case SELECTION:
     	 	 if(count != 0)
     	 	 { 	 
           get_colours(srf, s1, s2, s3, s4);
     	 	 	 ttk_line (srf, 0, ((h-HEADER)/4)*3, w, ((h-HEADER)/4)*3, ttk_makecol(WHITE));
     	 	 	 sprintf(str, "%d", count);
     	 	 	 pz_vector_string (srf, str, w-8, 2, 6, 8, 1, ttk_makecol(WHITE));
     	 	 	 		
           ttk_fillellipse (srf, ((w/6)*2)-((w/6)/2)-6, (w/6)+10, (w/6)/2, (w/6)/2, col1);
           ttk_fillellipse (srf, ((w/6)*3)-((w/6)/2)-1, (w/6)+10, (w/6)/2, (w/6)/2, col2);
           ttk_fillellipse (srf, ((w/6)*4)-((w/6)/2)+4, (w/6)+10, (w/6)/2, (w/6)/2, col3);
           ttk_fillellipse (srf, ((w/6)*5)-((w/6)/2)+9, (w/6)+10, (w/6)/2, (w/6)/2, col4);
         
           if(selection == 1) { ttk_ellipse (srf, ((w/6)*2)-((w/6)/2)-6, (w/6)+10, (w/6)/2, (w/6)/2, ttk_makecol(WHITE)); }
           else if(selection == 2) { ttk_ellipse (srf, ((w/6)*3)-((w/6)/2)-1, (w/6)+10, (w/6)/2, (w/6)/2, ttk_makecol(WHITE)); }
           else if(selection == 3) { ttk_ellipse (srf, ((w/6)*4)-((w/6)/2)+4, (w/6)+10, (w/6)/2, (w/6)/2, ttk_makecol(WHITE)); }
           else if(selection == 4) { ttk_ellipse (srf, ((w/6)*5)-((w/6)/2)+9, (w/6)+10, (w/6)/2, (w/6)/2, ttk_makecol(WHITE)); }
           	
           ttk_ellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 6, 6, ttk_makecol(WHITE));
           ttk_ellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 6, 6, ttk_makecol(WHITE));
           ttk_ellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 6, 6, ttk_makecol(WHITE));
           ttk_ellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 6, 6, ttk_makecol(WHITE));
           	
           if(paused == 1)
           {
           	 guess[0] = s1;
           	 guess[1] = s2;
           	 guess[2] = s3;
           	 guess[3] = s4;
           	 t=0;
  	         p=0;
  	         
  	         for(i=0;i<4;i++)
  	           if(guess[i] == hidden[i])
               {	           	
  		           t++;
  		           hidden[i] = hidden[i] - 8;
  		           guess[i] = guess[i] - 16;
  		         }
  		           
  	         for(i=0;i<4;i++)
  	           for(j=0;j<4;j++)
  		           if(guess[i] == hidden[j])
  		           {
  		             p++;
  		             hidden[j] = hidden[j] - 8;
  		             guess[i] = guess[i] - 16;
  		           }
  		         
  		       switch(t) {
  		       	 case 1:
  		       	 	 ttk_fillellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 switch(p){
                   case 3: ttk_fillellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
                   case 2: ttk_fillellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
                   case 1: ttk_fillellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE)); }
                 break;
  		       	 case 2:
  		       	 	 ttk_fillellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
  		       	 	 ttk_fillellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
  		       	 	 switch(p){
  		       	 	   case 2: ttk_fillellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
                   case 1: ttk_fillellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE)); }
                 break;
               case 3:
               	 ttk_fillellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
  		       	 	 ttk_fillellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 ttk_fillellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 if(p==1) ttk_fillellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
                 break;
               case 4:
               	 ttk_fillellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
  		       	 	 ttk_fillellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 ttk_fillellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 ttk_fillellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(GREEN));
                 break;
               default:
               	 switch(p){
               	   case 4: ttk_fillellipse (srf, (w/2)+18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
               	   case 3: ttk_fillellipse (srf, (w/2)+6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
               	   case 2: ttk_fillellipse (srf, (w/2)-6, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE));
               	   case 1: ttk_fillellipse (srf, (w/2)-18, (h-HEADER)-(((h-HEADER)/4)/2), 5, 6, ttk_makecol(BLUE)); }
               	 break;}
               	 
               	 for(i=0;i<4;i++)
  	               if(hidden[i] < 0)
  		               hidden[i] = hidden[i] + 8;
  		         
  		       if(t == 4) {
  		         	 menuframe = WON;
  		         	 wid->dirty++; }
  		         	 
  		       count--;
                                                         ttk_fillrect (srf, w-8, 0, w, h+10, ttk_makecol(BLACK));
                                                         sprintf(str, "%d", count);
     	 	       pz_vector_string (srf, str, w-8, 2, 6, 8, 1, ttk_makecol(WHITE));
  		       if(count == 0) {
  		       	 menuframe = LOSE;
  		       	 wid->dirty++;}
             paused=0;
  		     }
  		   }
  		   break;
  		   

  		 case WON:
  		 	  ttk_window_set_title(window, "YOU WIN!");
  		 	  pz_vector_string_center (srf, "Sweet.", w/2, (h-HEADER)/2, 10, 10, 1, ttk_makecol(WHITE));
  		    break;
  		    
  		 case LOSE:
          get_colours(srf, hidden[0], hidden[1], hidden[2], hidden[3]);
  		 	  ttk_window_set_title(window, "YOU LOSE!");
  		 	  pz_vector_string_center (srf, "the correct", w/2, 10, 6, 6, 1, ttk_makecol(WHITE));
  		 	  pz_vector_string_center (srf, "combination was:", w/2, 18, 6, 6, 1, ttk_makecol(WHITE));
  		 	  ttk_fillellipse (srf, ((w/6)*2)-((w/6)/2)-6, (h-HEADER)/2, (w/6)/2, (w/6)/2, col1);
          ttk_fillellipse (srf, ((w/6)*3)-((w/6)/2)-1, (h-HEADER)/2, (w/6)/2, (w/6)/2, col2);
          ttk_fillellipse (srf, ((w/6)*4)-((w/6)/2)+4, (h-HEADER)/2, (w/6)/2, (w/6)/2, col3);
          ttk_fillellipse (srf, ((w/6)*5)-((w/6)/2)+9, (h-HEADER)/2, (w/6)/2, (w/6)/2, col4);
  		 	  break;
     }
  }


  void get_colours(ttk_surface srf, int c1, int c2, int c3, int c4) {
    switch(c1) {
     	case 0: col1 = ttk_makecol(RED);    break;
     	case 1: col1 = ttk_makecol(BLUE);   break;
      case 2: col1 = ttk_makecol(YELLOW); break;
     	case 3: col1 = ttk_makecol(GREEN);  break; }
     	 	 	 	 
    switch(c2) {
     	case 0: col2 = ttk_makecol(RED);    break;
     	case 1: col2 = ttk_makecol(BLUE);   break;
     	case 2: col2 = ttk_makecol(YELLOW); break;
     	case 3: col2 = ttk_makecol(GREEN);  break; }
     	 	 	 	 
    switch(c3) {
     	case 0: col3 = ttk_makecol(RED);    break;
     	case 1: col3 = ttk_makecol(BLUE);   break;
     	case 2: col3 = ttk_makecol(YELLOW); break;
      case 3: col3 = ttk_makecol(GREEN);  break; }
     	 	 	 
    switch(c4) {
     	case 0: col4 = ttk_makecol(RED);    break;
      case 1: col4 = ttk_makecol(BLUE);   break;
     	case 2: col4 = ttk_makecol(YELLOW); break;
     	case 3: col4 = ttk_makecol(GREEN);  break; }	
  }


  void ChangeSelection( int side ) {
    switch(menuframe) {
  	case INTRO:
  		break;
  	case SELECTION:
  		if(side == 1)
  			if(selection == 1)
  		    if(s1 < 3) s1++;
  		if(side == 0) 
  			if(selection == 1) 
  				if(s1 > 0) s1--;
  		if(side == 1)
  			if(selection == 2)
  		    if(s2 < 3) s2++;
  		if(side == 0) 
  			if(selection == 2) 
  				if(s2 > 0) s2--;
  		if(side == 1)
  			if(selection == 3)
  		    if(s3 < 3) s3++;
  		if(side == 0) 
  			if(selection == 3) 
  				if(s3 > 0) s3--;
  		if(side == 1)
  			if(selection == 4)
  		    if(s4 < 3) s4++;
  		if(side == 0) 
  			if(selection == 4) 
  				if(s4 > 0) s4--;
  		paused=0;
  		wid->dirty++;
  		break;
  	case WON:
  		break;
  	case LOSE:
  		break;
  	}

  }

  void ConfirmSelection() {
    switch(menuframe) {
  	case INTRO:
  		menuframe = SELECTION;
  		break;
  	case SELECTION:
  		paused=1;
  		break;
  	case WON:
  		break;
  	case LOSE:
  		break;
  	}

  	wid->dirty++;
  }

  void reset_mastermind() {
  	 count = 6;
  	 paused = 0;
  	 selection = 1;
  	 s1=0, s2=1, s3=2, s4=3;
  }


  static int event_mastermind (PzEvent *e) {

  int ret = TTK_EV_CLICK;

  switch(e->type) {

     case PZ_EVENT_SCROLL:
       TTK_SCROLLMOD( e->arg, 10);
        if( e->arg > 0){ChangeSelection( 1 );}
        else {ChangeSelection( 0 );}
         break;

     case PZ_EVENT_BUTTON_DOWN:
        switch (e->arg) {
        case PZ_BUTTON_ACTION:
        	 ConfirmSelection();
        	 wid->dirty++;
           break;

        case PZ_BUTTON_MENU:
           reset_mastermind();
        	 pz_close_window(e->wid->win);
           break;

        case PZ_BUTTON_NEXT:
        	 if(selection < 4) selection++;
        	 wid->dirty++;
           break;

        case PZ_BUTTON_PREVIOUS:
        	 if(selection > 1) selection--;
        	 wid->dirty++;
           break;
           
        default:
           ret = TTK_EV_UNUSED; //do nothing if not buttons pressed
        }
        break;

     default:
        ret = TTK_EV_UNUSED; //do nothing
        break;

     }
     return ret;

  }

  PzWindow *new_mastermind_window() {
  	  menuframe = INTRO;
  	  srandom((unsigned)time(NULL));
  	  
      window = pz_new_window ("MasterMind", PZ_WINDOW_NORMAL);
      wid = pz_add_widget(window, draw_mastermind, event_mastermind);
      return pz_finish_window (window);
  }

  void cleanup_mastermind() {
  	  printf("I lost this time...");
      reset_mastermind();
  }

  void init_mastermind() {
      w =ttk_screen->w;
      h =ttk_screen->h;
      bpp =ttk_screen->bpp;
   
      module = pz_register_module ("mastermind", cleanup_mastermind);
      pz_menu_add_action ("/Extras/Games/MasterMind", new_mastermind_window);

  }

  PZ_MOD_INIT (init_mastermind)
