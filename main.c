
#include "allegro.h"

//create some colors
#define WHITE makecol(255,255,255)
#define BLACK makecol(0,0,0)
#define RED makecol(255,0,0)
#define GREEN makecol(0,255,0)
#define BLUE makecol(0,0,255)

//variables
int mousex, mousey, leftClick;
int leftClickBeingHeld;
int currentKey, scancode, ascii;
int startScreenMode = 1;
int mainMenuMode = 0;
int gameMode = 0;
int endScreenMode = 0;
int inputIndex;

char input[10];
PACKFILE *currentFile;
MIDI *music;
SAMPLE *SoundEffects[2];
int musicMode = 0; 
int gamePlayMode = 0; 
int lifeline = 3;
int points = 0;

//bitmaps
BITMAP *buffer;
BITMAP* buffers;

//functions
void clearScreen()
{
	rectfill(buffer, 0, 0, 640, 480, BLACK);
}

void updateGameInfo() 
{
	textout_ex(buffer, font, "DodgeIT (ESC to quit)", 0, 1, WHITE, 0);	
}

void updateGameInfos()
{
	textout_ex(buffers, font, "DodgeIT (ESC to quit)", 0, 1, WHITE, 0);
}

void clearScreens() 
{
	rectfill(buffers, 0, 0, 640, 480, BLACK);
}

void getInputFromKeyboard() 
{
	currentKey = readkey();
	scancode = (currentKey >> 8);
	ascii = scancode_to_ascii(scancode);

	//number
	if (ascii > 47 && ascii < 58 && inputIndex < 10)
	{
		input[inputIndex] = (char)ascii;
		inputIndex++;
	}

	//backspace
	else if (scancode == 63 || scancode == 77)
	{
		if (inputIndex > 0) 
		{
			inputIndex--;
			input[inputIndex] = 0;
		}
	}
}

void loadBackgroundImage(char *filename) 
{
	BITMAP *background;
	background = load_bitmap(filename, NULL);
	if (!background) 
	{
		allegro_message("couldnt load bg");
		return;
	}
	blit(background, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	destroy_bitmap(background);
}

void loadBackgroundImages(char* filename) 
{
	BITMAP* background;
	background = load_bitmap(filename, NULL);
	if (!background) 
	{
		allegro_message("couldnt load bg");
		return;
	}
	blit(background, buffers, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	destroy_bitmap(background);
}

int maxi(int a, int b)
{
	if (a >= b)
		return a;

	else
		return b;
}

int mini(int a, int b)
{
	if (a <= b)
		return a;
	else
		return b;
}



//pixel perfect method for collision detection 
int pixel_perfect_colision(int x1, int y1, BITMAP * obj1, int x2, int y2, BITMAP * obj2)
{
	int i, j;
	int collision = FALSE;

	//if rectangles intersect 
	if (!((x1 > x2 + obj2->w) || (y1 > y2 + obj2->h) || (x2 > x1 + obj1->w) || (y2 > y1 + obj1->h)))
	{

		int top = maxi(y1, y2);
		int low = mini(y1 + obj1->h, y2 + obj2->h);
		int left = maxi(x1, x2);
		int right = mini(x1 + obj1->w, x2 + obj2->w);

		for (i = top; i < low && !collision; i++)
		{
			for (j = left; j < right && !collision; j++)
			{
				if (getpixel(obj1, j - x1, i - y1) != makecol(255, 0, 255) && getpixel(obj2, j - x2, i - y2) != makecol(255, 0, 255)) {
					collision = TRUE;
				}
				
			}
		}
	}

	return collision;
}

//game reset
void resetGame()
{
	startScreenMode = 0;
	mainMenuMode = 0;
	gamePlayMode = 1; 
}


int main()
{
	allegro_init();
	set_window_title("DodgeIT");
	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
	install_keyboard();
	install_mouse();
	install_timer();
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
	buffer = create_bitmap(640, 480);
	buffers = create_bitmap(640, 480);

	set_mouse_sprite_focus(15, 15);
	show_mouse(screen);
	music = load_midi("houseofrisinsun.mid");
	SoundEffects[0] = load_sample("clapping.wav"); //winning answer
	SoundEffects[1] = load_sample("explode.wav"); //losing or dieing answer
	play_midi(music, 0);

	//create bitmaps
	BITMAP* boneco = load_bitmap("boneco.bmp", NULL);
	BITMAP* sky = load_bitmap("sky.bmp", NULL);
	BITMAP* montanha = load_bitmap("montanha.bmp", NULL);
	BITMAP* dragon = load_bitmap("dragon.bmp", NULL);
	BITMAP* Vikings = load_bitmap("Vikings.bmp", NULL);
	BITMAP* scientist = load_bitmap("madScientist.bmp", NULL);
	BITMAP* space = load_bitmap("space.bmp", NULL);
	BITMAP* enemy1 = load_bitmap("enemy1.bmp", NULL);
	BITMAP* Vikings_smallenemies_014 = load_bitmap("Vikings_smallenemies_014.bmp", NULL);

	BITMAP* Vikings_mediumenemies_01 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);
	BITMAP* Vikings_mediumenemies_02 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);
	BITMAP* Vikings_mediumenemies_03 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);
	BITMAP* Vikings_mediumenemies_04 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);
	BITMAP* Vikings_mediumenemies_05 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);
	BITMAP* Vikings_mediumenemies_06 = load_bitmap("Vikings_mediumenemies_013.bmp", NULL);


	//local variable    
	float teta = 0.0;


	while (!key[KEY_ESC])
	{
		clearScreen();
		updateGameInfo();
		mousex = mouse_x;
		mousey = mouse_y;
		leftClick = (mouse_b & 1);
		
		if (startScreenMode == 1)
		{
			//print title screen
			loadBackgroundImage("Introduction.bmp");

			//instructions			
			textout_centre_ex(buffer, font, "To turn off/on the music PRESS CTRL+M", SCREEN_W / 2, 22 + text_height(font), BLACK, WHITE);
			textout_centre_ex(buffer, font, "INSTRUCTIONS: Dodge your enemies and collect all the astroid stars to win.", 320, 390, BLACK, WHITE);
			textout_centre_ex(buffer, font, "Press ENTER to begin", SCREEN_W / 2, SCREEN_H / 2 + 100, BLACK, WHITE);
			textout_centre_ex(buffer, font, "Press ESC to terminate", SCREEN_W / 2, SCREEN_H / 2 + 200, BLACK, WHITE);

			blit(buffer, screen, 0, 0, 0, 0, 640, 480);

			//by default musicMode = 0 which means music 
			if (key[KEY_LCONTROL] && key[KEY_M] || key[KEY_RCONTROL] && key[KEY_M])
			{
				//if music is currently on then turn it off
				if (musicMode == 0)
				{
					musicMode = 1;
					stop_midi();
				}

				//if music is currently off then turn it on 
				if (musicMode == 1)
				{
					musicMode = 0;
					music = load_midi("houseofrisinsun.mid");
					play_midi(music, 0);
				}

			}

			if (key[KEY_ENTER])
			{
				startScreenMode = 0;
				mainMenuMode = 1;
				//clearScreen();
			}

		}


		//else if in main menu mode, do main menu behavior
		if (mainMenuMode == 1)
		{
			textout_ex(sky, font, "DodgeIT (ESC to quit)", 0, 1, BLACK, WHITE);
			textout_ex(sky, font, "Press Ctrl+H for Help", 460, 1, BLACK, WHITE);

			if (key[KEY_LCONTROL] && key[KEY_H] || key[KEY_RCONTROL] && key[KEY_H])
			{
				textout_ex(sky, font, "Moving Your Sprites: ", 200, 35, BLACK, WHITE);
				textout_ex(sky, font, " > to move right ", 200, 45, BLACK, WHITE);
				textout_ex(sky, font, " < to move left ", 200, 55, BLACK, WHITE);
				textout_ex(sky, font, " ^ to move up ", 200, 65, BLACK, WHITE);
				textout_ex(sky, font, " v to move down ", 200, 75, BLACK, WHITE);

				textout_ex(sky, font, " Lifelines will reduce upon colliding with enemies.", 200, 85, BLACK, WHITE);
			}


			//Note: Asking user to select their sprite from variety of the options available 
			textout_centre_ex(montanha, font, "Select Your Player", SCREEN_W / 2, 120 + text_height(font), BLACK, WHITE);

			//draw
			//Reference: https://rpg.hamsterrepublic.com/ohrrpgce/Free_Sprites
			set_trans_blender(225, 225, 255, 128);

			draw_sprite(buffers, sky, 0, 0);
			draw_sprite(buffers, montanha, 0, 0);
			draw_sprite_ex(buffers, boneco, 150, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);
			textout_centre_ex(montanha, font, "Boneco", 180, 410, BLACK, WHITE);

			draw_sprite_ex(buffers, dragon, 250, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);
			textout_centre_ex(montanha, font, "Dragon", 280, 410, BLACK, WHITE);

			draw_sprite_ex(buffers, Vikings, 350, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);
			textout_centre_ex(montanha, font, "Viking", 380, 390, BLACK, WHITE);

			draw_sprite_ex(buffers, scientist, 450, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);
			textout_centre_ex(montanha, font, "Scientist", 480, 390, BLACK, WHITE);

			//selection
			textout_centre_ex(montanha, font, "Press F1 to select: Boneco", SCREEN_W / 2, SCREEN_H / 2 + 200, BLACK, WHITE);
			textout_centre_ex(montanha, font, "Press F2 to select: Dragon", SCREEN_W / 2, SCREEN_H / 2 + 210, BLACK, WHITE);
			textout_centre_ex(montanha, font, "Press F3 to select: Viking", SCREEN_W / 2, SCREEN_H / 2 + 220, BLACK, WHITE);
			textout_centre_ex(montanha, font, "Press F4 to select: Scientist", SCREEN_W / 2, SCREEN_H / 2 + 230, BLACK, WHITE);


			draw_sprite(screen, buffers, 0, 0);
			clear(buffers);


			//if selected F1: Boneco
			if (key[KEY_F1]) 
			{

				mainMenuMode = 0;
				gamePlayMode = 1;
				clearScreens();
			}


			//if selected F2: Dragon
			else if (key[KEY_F2]) 
			{

				mainMenuMode = 0;
				gamePlayMode = 2;
				clearScreens();

			}


			//if selected F3: Viking
			else if (key[KEY_F3]) 
			{

				mainMenuMode = 0;
				gamePlayMode = 3;
				clearScreens();

			}


			//if selected F4: Scientist
			else if (key[KEY_F4]) 
			{

				mainMenuMode = 0;
				gamePlayMode = 4;
				clearScreens();

			}

		}


			   		 	  	  
		//Game Play Modes Based on 
		if (gamePlayMode == 1)
		{
			//set buffer
			draw_sprite(buffers, space, 0, 0);


			//give animation to person ability to move up/down (up and down only half way)/left/right
			int x = 150, y = 330;


			while (gamePlayMode == 1)
			{
				//need to clear 
				clearScreens();

				//aliens move around the planet (2-3) and player has to catch coins without crashing 
				draw_sprite(buffers, space, 0, 0);


				teta += 0.2;


				//stars to collect
				rotate_sprite(buffers, Vikings_mediumenemies_01, 160, 250, ftofix(teta));
				rotate_sprite(buffers, Vikings_mediumenemies_02, 300, 300, ftofix(teta));
				rotate_sprite(buffers, Vikings_mediumenemies_03, 400, 350, ftofix(teta));
				rotate_sprite(buffers, Vikings_mediumenemies_04, 380, 200, ftofix(teta));
				rotate_sprite(buffers, Vikings_mediumenemies_05, 550, 270, ftofix(teta));
				rotate_sprite(buffers, Vikings_mediumenemies_06, 560, 320, ftofix(teta));



				//enemies to dodge
				//screen (640, between 150 and 480)
				rotate_sprite(buffers, Vikings_smallenemies_014, 100, 250, ftofix(teta));
				rotate_sprite(buffers, Vikings_smallenemies_014, 200, 300, ftofix(teta));
				rotate_sprite(buffers, Vikings_smallenemies_014, 300, 350, ftofix(teta));
				rotate_sprite(buffers, Vikings_smallenemies_014, 350, 200, ftofix(teta));
				rotate_sprite(buffers, Vikings_smallenemies_014, 500, 270, ftofix(teta));
				rotate_sprite(buffers, Vikings_smallenemies_014, 600, 320, ftofix(teta));




				//ESCAPE to quit
				textout_ex(space, font, "DodgeIT (ESC to quit)", 10, 1, BLACK, WHITE);
				textprintf_ex(space, font, 10, 15, BLACK, WHITE, " Health: %i", lifeline);
				textprintf_ex(space, font, 10, 30, BLACK, WHITE, " Points: %i", points);



				if (key[KEY_ESC])
				{

					break;
				}

				//dealing with collisions 
				//enemies
				if (pixel_perfect_colision(x, y, boneco, 100, 250, Vikings_smallenemies_014) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 200, 300, Vikings_smallenemies_014) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 300, 350, Vikings_smallenemies_014) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 350, 200, Vikings_smallenemies_014) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 500, 270, Vikings_smallenemies_014) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 600, 320, Vikings_smallenemies_014) == TRUE) {

					lifeline--;

					break;
				}


				// dealing with collision with coins 
				//add points and destroy the sprite 
				if (pixel_perfect_colision(x, y, boneco, 160, 250, Vikings_mediumenemies_01) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 300, 300, Vikings_mediumenemies_02) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 400, 350, Vikings_mediumenemies_03) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 380, 200, Vikings_mediumenemies_04) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 550, 270, Vikings_mediumenemies_05) == TRUE ||
					pixel_perfect_colision(x, y, boneco, 560, 320, Vikings_mediumenemies_06) == TRUE) {

					points++;
					
					draw_sprite(buffers, space, 0, 0);

					if (points == 6)
					{
						textout_ex(space, font, "You Won. Press [ESC] and play again. Try reaching to all the astroids.", 60, 150, BLACK, WHITE);
					}

					break;
				}




				if (key[KEY_LEFT]) {
					x = x - 1;

					if (x == 0 || x == 640)
					{
						x = 150;
						//gamePlayMode == 0;
					}
				}

				if (key[KEY_RIGHT]) {
					x = x + 1;

					if (x == 0 || x == 640)
					{
						x = 150;
						//gamePlayMode == 0;
					}

				}

				if (key[KEY_UP]) {
					y = y - 1;

					if (y == 0 || y <= 150)
					{
						y = 330;
						//gamePlayMode == 0;
					}
				}

				if (key[KEY_DOWN]) {
					y = y + 1;

					if (y == 0 || y <= 150 || y >= 480)
					{
						y = 330;
						//gamePlayMode == 0;
					}
				}

				draw_sprite(buffers, boneco, x, y); //I think this allows the picture to move freely on the bmp





				draw_sprite(screen, buffers, 0, 0);

				if (lifeline == 0)
				{
					textout_ex(space, font, "Game Over. Press ESC and Try Again.", 250, 1, BLACK, WHITE);
					break;
				}
			}
		}

		if (gamePlayMode == 2) 
		{

			//set buffer
			draw_sprite(buffers, space, 0, 0);

			//give animation to person ability to move up/down (up and down only half way)/left/right
			int x = 150, y = 330;

			while (gamePlayMode == 2)
			{
						//need to clear 
						clearScreens();

						//aliens move around the planet (2-3) and player has to catch coins without crashing 
						draw_sprite(buffers, space, 0, 0);


						teta += 0.2;


						//stars to collect
						rotate_sprite(buffers, Vikings_mediumenemies_01, 160, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_02, 300, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_03, 400, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_04, 380, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_05, 550, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_06, 560, 320, ftofix(teta));



						//enemies to dodge
						//screen (640, between 150 and 480)
						rotate_sprite(buffers, Vikings_smallenemies_014, 100, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 200, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 300, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 350, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 500, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 600, 320, ftofix(teta));




						//ESCAPE to quit
						textout_ex(space, font, "DodgeIT (ESC to quit)", 10, 1, BLACK, WHITE);
						textprintf_ex(space, font, 10, 15, BLACK, WHITE, " Health: %i", lifeline);
						textprintf_ex(space, font, 10, 30, BLACK, WHITE, " Points: %i", points);


						if (key[KEY_ESC])
						{
							break;
						}


						//dealing with collisions 
						//enemies
						if (pixel_perfect_colision(x, y, dragon, 100, 250, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 200, 300, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 300, 350, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 350, 200, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 500, 270, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 600, 320, Vikings_smallenemies_014) == TRUE) {

							lifeline--;

							break;
						}


						// dealing with collision with coins 
						//add points and destroy the sprite 
						if (pixel_perfect_colision(x, y, dragon, 160, 250, Vikings_mediumenemies_01) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 300, 300, Vikings_mediumenemies_02) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 400, 350, Vikings_mediumenemies_03) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 380, 200, Vikings_mediumenemies_04) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 550, 270, Vikings_mediumenemies_05) == TRUE ||
							pixel_perfect_colision(x, y, dragon, 560, 320, Vikings_mediumenemies_06) == TRUE) {

							points++;

							draw_sprite(buffers, space, 0, 0);

							if (points == 6)
							{
								textout_ex(space, font, "You Won. Press [ESC] and play again. Try reaching to all the astroids.", 60, 150, BLACK, WHITE);
							}

							break;
						}




						if (key[KEY_LEFT]) 
						{
							x = x - 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_RIGHT]) 
						{
							x = x + 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}

						}

						if (key[KEY_UP]) 
						{
							y = y - 1;

							if (y == 0 || y <= 150)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_DOWN]) 
						{
							y = y + 1;

							if (y == 0 || y <= 150 || y >= 480)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						draw_sprite(buffers, dragon, x, y); //I think this allows the picture to move freely on the bmp

											   						 
						draw_sprite(screen, buffers, 0, 0);

						if (lifeline == 0)
						{
							textout_ex(space, font, "Game Over. Press ESC and Try Again.", 250, 1, BLACK, WHITE);
							break;
						}
			}
		}












		if (gamePlayMode == 3) 
		{
			//set buffer
			draw_sprite(buffers, space, 0, 0);

			//give animation to person ability to move up/down (up and down only half way)/left/right
			int x = 150, y = 330;

			while (gamePlayMode == 3)
					{
						//need to clear 
						clearScreens();

						//aliens move around the planet (2-3) and player has to catch coins without crashing 
						draw_sprite(buffers, space, 0, 0);


						teta += 0.2;


						//stars to collect
						rotate_sprite(buffers, Vikings_mediumenemies_01, 160, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_02, 300, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_03, 400, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_04, 380, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_05, 550, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_06, 560, 320, ftofix(teta));



						//enemies to dodge
						//screen (640, between 150 and 480)
						rotate_sprite(buffers, Vikings_smallenemies_014, 100, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 200, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 300, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 350, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 500, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 600, 320, ftofix(teta));




						//ESCAPE to quit
						textout_ex(space, font, "DodgeIT (ESC to quit)", 10, 1, BLACK, WHITE);
						textprintf_ex(space, font, 10, 15, BLACK, WHITE, " Health: %i", lifeline);
						textprintf_ex(space, font, 10, 30, BLACK, WHITE, " Points: %i", points);


						if (key[KEY_ESC])
						{
							break;
						}


						//dealing with collisions 
						//enemies
						if (pixel_perfect_colision(x, y, Vikings, 100, 250, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 200, 300, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 300, 350, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 350, 200, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 500, 270, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 600, 320, Vikings_smallenemies_014) == TRUE) {

							lifeline--;

							break;
						}


						// dealing with collision with coins 
						//add points and destroy the sprite 
						if (pixel_perfect_colision(x, y, Vikings, 160, 250, Vikings_mediumenemies_01) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 300, 300, Vikings_mediumenemies_02) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 400, 350, Vikings_mediumenemies_03) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 380, 200, Vikings_mediumenemies_04) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 550, 270, Vikings_mediumenemies_05) == TRUE ||
							pixel_perfect_colision(x, y, Vikings, 560, 320, Vikings_mediumenemies_06) == TRUE) {

							points++;

							draw_sprite(buffers, space, 0, 0);

							if (points == 6)
							{
								textout_ex(space, font, "You Won. Press [ESC] and play again. Try reaching to all the astroids.", 60, 150, BLACK, WHITE);
							}

							break;
						}




						if (key[KEY_LEFT])
						{
							x = x - 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_RIGHT])
						{
							x = x + 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}

						}

						if (key[KEY_UP])
						{
							y = y - 1;

							if (y == 0 || y <= 150)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_DOWN])
						{
							y = y + 1;

							if (y == 0 || y <= 150 || y >= 480)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						draw_sprite(buffers, Vikings, x, y); //I think this allows the picture to move freely on the bmp


						draw_sprite(screen, buffers, 0, 0);

						if (lifeline == 0)
						{
							textout_ex(space, font, "Game Over. Press ESC and Try Again.", 250, 1, BLACK, WHITE);
							break;
						}
					}
		}
		


			   		 	  	  
		if (gamePlayMode == 4) 		
		{
					//set buffer
					draw_sprite(buffers, space, 0, 0);

					//give animation to person ability to move up/down (up and down only half way)/left/right
					int x = 150, y = 330;

					while (gamePlayMode == 4)
					{
						//need to clear 
						clearScreens();

						//aliens move around the planet (2-3) and player has to catch coins without crashing 
						draw_sprite(buffers, space, 0, 0);


						teta += 0.2;


						//stars to collect
						rotate_sprite(buffers, Vikings_mediumenemies_01, 160, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_02, 300, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_03, 400, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_04, 380, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_05, 550, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_mediumenemies_06, 560, 320, ftofix(teta));



						//enemies to dodge
						//screen (640, between 150 and 480)
						rotate_sprite(buffers, Vikings_smallenemies_014, 100, 250, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 200, 300, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 300, 350, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 350, 200, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 500, 270, ftofix(teta));
						rotate_sprite(buffers, Vikings_smallenemies_014, 600, 320, ftofix(teta));




						//ESCAPE to quit
						textout_ex(space, font, "DodgeIT (ESC to quit)", 10, 1, BLACK, WHITE);
						textprintf_ex(space, font, 10, 15, BLACK, WHITE, " Health: %i", lifeline);
						textprintf_ex(space, font, 10, 30, BLACK, WHITE, " Points: %i", points);


						if (key[KEY_ESC])
						{
							break;
						}


						//dealing with collisions 
						//enemies
						if (pixel_perfect_colision(x, y, scientist, 100, 250, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 200, 300, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 300, 350, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 350, 200, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 500, 270, Vikings_smallenemies_014) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 600, 320, Vikings_smallenemies_014) == TRUE) {

							lifeline--;

							break;
						}


						// dealing with collision with coins 
						//add points and destroy the sprite 
						if (pixel_perfect_colision(x, y, scientist, 160, 250, Vikings_mediumenemies_01) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 300, 300, Vikings_mediumenemies_02) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 400, 350, Vikings_mediumenemies_03) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 380, 200, Vikings_mediumenemies_04) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 550, 270, Vikings_mediumenemies_05) == TRUE ||
							pixel_perfect_colision(x, y, scientist, 560, 320, Vikings_mediumenemies_06) == TRUE) {

							points++;

							draw_sprite(buffers, space, 0, 0);

							if (points == 6)
							{
								textout_ex(space, font, "You Won. Press [ESC] and play again. Try reaching to all the astroids.", 60, 150, BLACK, WHITE);
							}

							break;
						}




						if (key[KEY_LEFT])
						{
							x = x - 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_RIGHT])
						{
							x = x + 1;

							if (x == 0 || x == 640)
							{
								x = 150;
								//gamePlayMode == 0;
							}

						}

						if (key[KEY_UP])
						{
							y = y - 1;

							if (y == 0 || y <= 150)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						if (key[KEY_DOWN])
						{
							y = y + 1;

							if (y == 0 || y <= 150 || y >= 480)
							{
								y = 330;
								//gamePlayMode == 0;
							}
						}

						draw_sprite(buffers, scientist, x, y); //I think this allows the picture to move freely on the bmp


						draw_sprite(screen, buffers, 0, 0);

						if (lifeline == 0)
						{
							textout_ex(space, font, "Game Over. Press ESC and Try Again.", 250, 1, BLACK, WHITE);
							break;
						}
					}
		}

		
	}

	stop_midi();
	destroy_midi(music);
	remove_sound();
	set_mouse_sprite(NULL);


	//destroy
	destroy_bitmap(buffer);
	destroy_bitmap(boneco);
	destroy_bitmap(sky);
	destroy_bitmap(montanha);
	destroy_bitmap(space);
	destroy_bitmap(buffers);
	destroy_bitmap(scientist);
	destroy_bitmap(dragon);
	destroy_bitmap(Vikings);


	allegro_exit();
}
END_OF_MAIN()


