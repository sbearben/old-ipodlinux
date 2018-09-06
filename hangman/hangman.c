	/*
	 * Copyright (C) 2006 Armon Khosravi (miz dawg)
	 * Copyright (C) 2005 Zachary Howe
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
	#include <time.h>
	#include <unistd.h>
	#include <termios.h>
	#include <memory.h>
	#include <ctype.h>
	#include "pz.h"

	#define HEADER ttk_screen->wy
	#define HWH (ttk_screen->h-ttk_screen->wy)
	#define MAX_WORD_LEN 255
	#define SCREEN_HEIGHT 25
	#define MAX_GUESSES 11 
	#define BLANK_CHAR '*'

	static PzModule *module;
	static TWindow *window;
	static TWidget *wid;
	static PzConfig *config;

	/* simple struct for chained list*/
	struct maillon {
		char *word;
		struct maillon *suivant;
	};

	typedef struct maillon Maillon;

	Maillon *word_list_pointer = NULL;
	int	list_size = 0;

	static const char *player_options[] = {"1","2",0};
	static int hitaction=0, enter=0, doonce=0, confirm=0, once=0;
	static int w, h;
	static int count, lcount=0;
	static char letter='A';
	static char word[MAX_WORD_LEN+1], *rword;
	static char display[MAX_WORD_LEN+1];

	void draw_hangman();
	void init_word_list();
	char *getrandomword();
	void get_user_word(ttk_surface srf);
	int checkforwin(char *word, char *display);
	void guess(char *word);
	void reset_hangman();



	void hangman_start (PzWidget *wid, ttk_surface srf)
	{
		 int players;
	   int width, height;
	   int oldblanks, len, i;
		 char str[MAX_WORD_LEN+1];
		 
		 if(once==0) {
		   init_word_list();
		   once++;
		 }
		 
		 ttk_fillrect (srf, 0, 0, w, h, ttk_makecol(GREY));
		 players = pz_get_int_setting(config, 1)+1;
		 if(players==1) confirm=1;
		 
		 if(players==2 && confirm==0)
		   get_user_word(srf);
		 else if((players==1 || players==2) && confirm==1)
		 {
		   if(doonce==0)
		   {
		 	   if(players==1){
			     rword = getrandomword();
	         len = strlen(rword);}
			   else len = strlen(word);
	       for (i = 0; i < len; i++) {
	         display[i] = BLANK_CHAR; }
	       display[i] = '\0';
	       count = 0;
	       oldblanks = len;
			   doonce=1;
		   }
			
			
			
			 if(count < MAX_GUESSES)
			 {
				 draw_hangman(srf);
			   height = ttk_text_height (ttk_menufont);
				
				 sprintf(str, "Guesses Left: %d", MAX_GUESSES-count);
				 width = ttk_text_width (ttk_menufont, str);
				 ttk_text (srf, ttk_menufont, (w-width)-5, HWH-20, ttk_ap_getx("window.fg")->color, str);
				
				 sprintf(str, "%s", display);
	       width = ttk_text_width (ttk_menufont, str);
				 ttk_text (srf, ttk_menufont, (w/4)-(width/2), HWH/2, ttk_ap_getx("window.fg")->color, str);
				
				 sprintf(str, "Input: \"%c\"", tolower(letter));
				 width = ttk_text_width (ttk_menufont, str);
				 ttk_rect (srf, 18, 18, (20+width)+5, (20+height)+5, ttk_makecol(BLACK));
		     ttk_text (srf, ttk_menufont, 22, 22, ttk_ap_getx("window.fg")->color, str);
		    
		     if(hitaction==1)
		     {
		    	 if(players==1) guess(rword);
		    	 else guess(word);
				   hitaction=0;
		     }
		     wid->dirty++;
		   }
		  
		   if(checkforwin(word, display) == 0 || checkforwin(rword, display) == 0)
		   {
	       ttk_fillrect (srf, 0, 0, w, h, ttk_makecol(GREY));
		  	 ttk_window_set_title(window, "YOU WIN!");
		  	 if(players==1) sprintf(str, "%s", rword);
		  	 if(players==2) sprintf(str, "%s", word);
	       width = ttk_text_width (ttk_menufont, "The word was:");
				 ttk_text (srf, ttk_menufont, (w/2)-(width/2), HWH/2-20, ttk_ap_getx("window.fg")->color, "The word was:");
	       width = ttk_text_width (ttk_menufont, str);
	       ttk_text (srf, ttk_menufont, (w/2)-(width/2), HWH/2, ttk_ap_getx("window.fg")->color, str);
		   }
		  
		   else if((checkforwin(word, display) != 0 || checkforwin(rword, display) != 0) && count == MAX_GUESSES)
		   {
		   	 ttk_fillrect (srf, 0, 0, w, h, ttk_makecol(GREY));
	       ttk_window_set_title(window, "YOU LOSE!");
		  	 if(players==1) sprintf(str, "%s", rword);
		  	 if(players==2) sprintf(str, "%s", word);
	       width = ttk_text_width (ttk_menufont, "The correct word was:");
				 ttk_text (srf, ttk_menufont, (w/2)-(width/2), HWH/2-20, ttk_ap_getx("window.fg")->color, "The correct word was:");
	       width = ttk_text_width (ttk_menufont, str);
	       ttk_text (srf, ttk_menufont, (w/2)-(width/2), HWH/2, ttk_ap_getx("window.fg")->color, str);
	                         
		   }
	   }
	}


	void get_user_word(ttk_surface srf)
	{
		int width;
		char str[MAX_WORD_LEN+1];
		
		sprintf(str, "Input: \"%c\"", tolower(letter));
	  width = ttk_text_width (ttk_menufont, str);
		ttk_text (srf, ttk_menufont, (w/2)-(width/2), 22, ttk_ap_getx("window.fg")->color, str);
		   
		sprintf(str, "%s", word);
		width = ttk_text_width (ttk_menufont, str);
		ttk_text (srf, ttk_menufont, (w/2)-(width/2), HWH/2, ttk_ap_getx("window.fg")->color, str);
			 
		if(enter==1)
		{
			word[lcount] = tolower(letter);
			lcount++;
	    enter=0;
	    wid->dirty++;
		}
	}


	void draw_hangman(ttk_surface srf)
	{
		 switch(MAX_GUESSES-count) 
		 {
	   	case 0:
	   		ttk_line (srf, ((w/4)*3)+1, HWH/2+1, ((w/4)*3)+12, (HWH/2)+15, ttk_makecol(BLACK));
	   	case 1:
	   		ttk_line (srf, ((w/4)*3)-1, HWH/2+1, ((w/4)*3)-12, (HWH/2)+15, ttk_makecol(BLACK));
	    case 2:
	    	ttk_line (srf, (w/4)*3, HWH/4+22, ((w/4)*3)+10, (HWH/4+22)+12, ttk_makecol(BLACK));
	    case 3:
	    	ttk_line (srf, (w/4)*3, HWH/4+22, ((w/4)*3)-10, (HWH/4+22)+12, ttk_makecol(BLACK));
	    case 4:	
	    	ttk_line (srf, (w/4)*3, HWH/4+20, (w/4)*3, HWH/2, ttk_makecol(BLACK));
	    case 5:
	    	ttk_ellipse (srf, (w/4)*3, HWH/4+15, 5, 5, ttk_makecol(BLACK));
	    case 6:
	    	ttk_line (srf, (w/4)*3, HWH/4, (w/4)*3, HWH/4+10, ttk_makecol(BLACK));
	    case 7:
	    	ttk_line (srf, (w/2)+15, (HWH/4)+10, (w/2)+25, HWH/4, ttk_makecol(BLACK));
	    case 8:
	    	ttk_line (srf, (w/2)+15, HWH/4, (w/4)*3, HWH/4, ttk_makecol(BLACK));
	    case 9:
	    	ttk_line (srf, (w/2)+15, (HWH/4)*3, (w/2)+15, HWH/4, ttk_makecol(BLACK));
	    case 10:
	    	ttk_line (srf, (w/2)+5, (HWH/4)*3, (w/2)+25, (HWH/4)*3, ttk_makecol(BLACK));
	    	break;
	    default:
	    	break;
	   }
	 }


	void init_word_list()
	{
		FILE *wordlist;
		char wordl[128];
		Maillon *maille,*file;
		
		printf("Loading word file... ");
		srand(time(0));
		wordlist = fopen(pz_module_get_datapath (module, "wordlist.txt"), "r");
		
		if (wordlist == NULL)
		{
			printf("Could not open word list file\n");
			pz_error("Could not open word list file");
			//exit(0);
		}
		
		while (fscanf(wordlist, "%s", wordl) != EOF)
		{
			maille = (Maillon*)malloc(sizeof(Maillon));
			if (maille == NULL)
			{
				printf("malloc failed, exiting\n");
				pz_error ("Malloc failed, exiting!");
				exit(0);
			}
			maille->word = (char*)malloc((strlen(wordl)+1)*sizeof(char));
			if (maille->word == NULL)
			{
				printf("malloc failed, exiting\n");
				pz_error ("Malloc failed, exiting!");
				exit(0);
			}
			strcpy(maille->word,wordl);
			if (word_list_pointer == NULL)
			{
				maille->suivant = NULL;
				word_list_pointer = maille;
			}
			else
			{
				maille->suivant = file;
			}
			file = maille;
			list_size++;
		}
		fclose(wordlist);
		word_list_pointer->suivant = file;
		word_list_pointer = file;
		printf("done\n");
	}   	


	char *getrandomword()
	{
		int rnd = rand() % list_size;
		printf("\n\nPicking random word... ");
		do {
			word_list_pointer = word_list_pointer->suivant;
		} while (--rnd > 0);
		printf("done\n");
		return word_list_pointer->word;
	}


	int checkforwin(char *word, char *display)
	{
		return strcmp(word, display);
	}


	void guess(char *word)
	{
	    int i, len;
	    int rcount=0;
	    len = strlen(word);

	    for(i=0;i<len;i++) {
	        if (word[i] == letter || word[i] == tolower(letter)) {
	            display[i] = tolower(letter);
	            rcount++;
	        }
	    }
	    if(rcount == 0) {
	    	//no letters uncovered
	    	count++; 
	    }
	    	
	    rcount=0;
	}


	void reset_hangman()
	{
	  int i;
		letter = 'A';
		count = 0;
		hitaction = 0;
		doonce = 0;
		enter = 0;
		confirm = 0;
		lcount = 0;

	  for(i=0;i<sizeof(word);i++)
	    word[i] = '\0';
	    
	  for(i=0;i<sizeof(rword);i++)
	    rword[i] = '\0';
	}


	static int event_hangman (PzEvent *e)
	{

	int ret = TTK_EV_CLICK;

	switch(e->type) {

	   case PZ_EVENT_SCROLL:
	     TTK_SCROLLMOD( e->arg, 10);
	      if( e->arg > 0){
	      	if((int)letter < 90) 
	      		(int)letter++;}
	      else {
	      	if((int)letter > 65)
	      	   (int)letter--;}
	      wid->dirty++;
	      break;

	   case PZ_EVENT_BUTTON_DOWN:
	      switch (e->arg) {
	      case PZ_BUTTON_ACTION:
	      	 if(confirm==1)
	      	   hitaction=1;
	      	 if(confirm==0)
	      	 	 enter=1;
	      	 wid->dirty++;
	         break;

	      case PZ_BUTTON_MENU:
	      	 pz_close_window(e->wid->win);
	         break;

	      case PZ_BUTTON_PLAY:
	         break;

	      case PZ_BUTTON_NEXT:
	      	 if(confirm==0) {
	      	 	 confirm=1;
	           letter='A'; }
	      	 	 wid->dirty++;
	         break;

	      case PZ_BUTTON_PREVIOUS:
	         if(confirm==0) {
	           lcount--;
	           word[lcount] = '\0'; }
	         wid->dirty++;
	         break;

	      case PZ_BUTTON_HOLD :   
	         break;
	         
	      default:
	         ret = TTK_EV_UNUSED;
	      }
	      break;

	   default:
	      ret = TTK_EV_UNUSED; //do nothing
	      break;

	   }
	   return ret;

	}

	PzWindow *new_hangman_window()
	{
		  reset_hangman();
		  srandom((unsigned)time(NULL));
		  
	    window = pz_new_window ("Hangman", PZ_WINDOW_NORMAL);
	    wid = pz_add_widget(window, hangman_start, event_hangman);
	    return pz_finish_window (window);
	}

	void cleanup_hangman()
	{
	    printf("Took her for granted...");
	    reset_hangman();
	}

	static int save_hangman_config(TWidget *this, int key, int time)
	{
		if(key == TTK_BUTTON_MENU){
			pz_save_config(config); 
		}
		return ttk_menu_button(this, key, time);
	}

	void init_hangman()
	{
		  w =ttk_screen->w;
	    h =ttk_screen->h;
	    
	    module = pz_register_module ("hangman", cleanup_hangman);
	    config = pz_load_config(pz_module_get_cfgpath(module, "hangman.conf"));
	    pz_menu_add_action ("/Extras/Games/Hangman/Start", new_hangman_window);
	    pz_menu_add_setting("/Extras/Games/Hangman/Players", 1, config, player_options);
	  ((TWidget *)pz_get_menu_item("/Extras/Games/Hangman")->data)->button = save_hangman_config;
	}

	PZ_MOD_INIT (init_hangman)
