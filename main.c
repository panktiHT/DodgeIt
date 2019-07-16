
#include "allegro.h"

//create some colors
#define WHITE makecol(255,255,255)
#define BLACK makecol(0,0,0)
#define RED makecol(255,0,0)
#define GREEN makecol(0,255,0)
#define BLUE makecol(0,0,255)
#define SMOKE makecol(140,130,120)
#define CORRECT 1
#define INCORRECT 0
#define N_A -1

int numberOfCorrectAnswers = 0;
int numberOfIncorrectAnswers = 0;
int mousex, mousey, leftClick;
int textcolor, bgcolor, buttoncolor;
int leftClickBeingHeld;
int currentKey, scancode, ascii;
int startScreenMode = 1;
int mainMenuMode = 0;
int selectChapterMode = 0;
int selectUnitMode = 0;
int selectNumberOfQuestionsMode = 0;
int numberOfQuestions;
int gameMode = 0;
int endScreenMode = 0;
int inputIndex;
int currentQuestion;
int newQuestionNeeded = 1;
int isFirstQuestion = 1;
int questionsDone[220];
char input[10];
char filename[14];
char textLine[200];
PACKFILE *currentFile;
MIDI *music;
SAMPLE *SoundEffects[2];
int musicMode = 0; 
int gamePlayMode = 0; 

typedef struct BUTTON
{
	char *buttonText;
	int isCorrectAnswer;
	int x1, x2, y1, y2; // area on screen where clicking will select this answer
}BUTTON;

BITMAP *buffer;
BUTTON buttons[20];

BITMAP* buffers;


void printScore() {
	textprintf_right_ex(buffer, font, SCREEN_W - 5, 1, WHITE, 0,
		"Correct: %d Incorrect: %d ", numberOfCorrectAnswers, numberOfIncorrectAnswers);
}
void correctAnswer() {
	numberOfCorrectAnswers++;
	printScore();
}
void incorrectAnswer() {
	numberOfIncorrectAnswers++;
	printScore();
}

typedef struct QUESTION
{
	char *questionText;
	BUTTON answers[4];
}QUESTION;

QUESTION questions[220];
int questionIndex = 0;

int isButtonSelected(BUTTON button) 
{
	if (leftClick && mousex > button.x1 && mousex < button.x2 && mousey > button.y1 && mousey < button.y2) {
		return 1;
	}
	return 0;
}

int isValueInArray(int val, int *arr, int size) {
	for (int i = 0; i < size; i++) {
		if (arr[i] == val)
			return 1;
	}
	return 0;
}
void fillArrayWithNegatives(int *arr, int size) {
	for (int i = 0; i < size; i++) {
		arr[i] = -1;
	}
}



//keep
void updateGameInfo() 
{
	textout_ex(buffer, font, "DodgeIT (ESC to quit)", 0, 1, WHITE, 0);
	printScore();
	rect(buffer, 0, 12, SCREEN_W - 2, SCREEN_H - 2, textcolor);
}

void updateGameInfos()
{
	textout_ex(buffers, font, "DodgeIT (ESC to quit)", 0, 1, WHITE, 0);
	printScore();
	rect(buffers, 0, 12, SCREEN_W - 2, SCREEN_H - 2, textcolor);
}

void clearScreen() {
	rectfill(buffer, 0, 0, 640, 480, BLACK);
}

void clearScreens() {
	rectfill(buffers, 0, 0, 640, 480, BLACK);
}

void getQuestionsFromChapterFile(char *fileLocation) {
	//load chapter file
	
	currentFile = pack_fopen(fileLocation, "r");
	while (!pack_feof(currentFile)) {
		//get question text
		pack_fgets(textLine, sizeof(textLine), currentFile);
		questions[questionIndex].questionText = (char*)malloc(sizeof(char) * (strlen(textLine) + 1));
		ustrzcpy(questions[questionIndex].questionText, sizeof(textLine), textLine);
		//get answers to question
		for (int index = 0; index < 4; index++) {
			pack_fgets(textLine, sizeof(textLine), currentFile);
			// ~ is the marker for correct answer, so check if it's there
			if (ustrstr(textLine, "~") != NULL) {
				uremove(textLine, -1); // trim correct marker off of the end
				questions[questionIndex].answers[index].buttonText = (char*)malloc(sizeof(char) * (strlen(textLine) + 1));
				ustrzcpy(questions[questionIndex].answers[index].buttonText, sizeof(textLine), textLine);
				questions[questionIndex].answers[index].isCorrectAnswer = CORRECT;
			}
			else {
				questions[questionIndex].answers[index].buttonText = (char*)malloc(sizeof(char) * (strlen(textLine) + 1));
				ustrzcpy(questions[questionIndex].answers[index].buttonText, sizeof(textLine), textLine);
				questions[questionIndex].answers[index].isCorrectAnswer = INCORRECT;
			}
		}
		questionIndex++;
	}
}

void getInputFromKeyboard() {
	currentKey = readkey();
	scancode = (currentKey >> 8);
	ascii = scancode_to_ascii(scancode);
	//number
	if (ascii > 47 && ascii < 58 && inputIndex < 10) {
		input[inputIndex] = (char)ascii;
		inputIndex++;
	}
	//backspace
	else if (scancode == 63 || scancode == 77) {
		if (inputIndex > 0) {
			inputIndex--;
			input[inputIndex] = 0;
		}
	}
}

void emptyCharArray(char *charArray, int size) {
	for (int i = 0; i < size; i++) {
		charArray[i] = 0;
	}
}

void wrapText(BITMAP *bmp, FONT *f, char *str, int x, int y, int color, int bg) {
	char currentline[400];
	emptyCharArray(currentline, 400);
	char nextline[400];
	emptyCharArray(nextline, 400);
	ustrzcpy(currentline, 400, str);
	while ((text_length(font, currentline) + x) > SCREEN_W) {
		uinsert(nextline, 0, ugetat(currentline, -1));
		uremove(currentline, -1);
	}
	textout_ex(bmp, f, currentline, x, y, color, bg);
	if (strlen(nextline) > 0) {
		wrapText(bmp, f, nextline, x, y + text_height(font) + 2, color, bg);
	}
}

BUTTON drawNewButton(int x1, int x2, int y1, int y2, int colour, char *buttonMessage, int isCorrect) {
	rectfill(buffer, x1, y1, x2, y2, colour);
	//write text next to button
	wrapText(buffer, font, buttonMessage, (int)(x2 + 5), (int)(y1 + y2) / 2, textcolor, bgcolor);
	//textout_ex(buffer, font, buttonMessage, (x2 + 5), (y1 + y2)/2, textcolor, 0);
	BUTTON button;
	button.buttonText = buttonMessage;
	button.x1 = x1;
	button.x2 = x2;
	button.y1 = y1;
	button.y2 = y2;
	button.isCorrectAnswer = isCorrect;
	return button;
}

void loadBackgroundImage(char *filename) {
	BITMAP *background;
	background = load_bitmap(filename, NULL);
	if (!background) {
		allegro_message("couldnt load bg");
		return;
	}
	blit(background, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	destroy_bitmap(background);
}


void loadBackgroundImages(char* filename) {
	BITMAP* background;
	background = load_bitmap(filename, NULL);
	if (!background) {
		allegro_message("couldnt load bg");
		return;
	}
	blit(background, buffers, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	destroy_bitmap(background);
}

int compareCharArrayToLiteral(char *charArray, char literal) {
	if (charArray[0] == literal) {
		for (int i = 1; i < ustrlen(charArray); i++) {
			if (charArray[i] != 0) {
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

int isInputDigit(char *arrayofchars) {
	for (int counter = 0; counter < ustrlen(arrayofchars); counter++) {
		if (!uisdigit(ugetat(arrayofchars, counter)))
			return 0;
	}
	return 1;
}

void resetGame() 
{
	numberOfCorrectAnswers = 0;
	numberOfIncorrectAnswers = 0;
	emptyCharArray(input, 10);
	emptyCharArray(filename, 14);
	set_mouse_sprite_focus(15, 15);
	fillArrayWithNegatives(questionsDone, 220);
	endScreenMode = 0;
	startScreenMode = 1;
	numberOfQuestions = 0;
	questionIndex = 0;
	inputIndex = 0;
	isFirstQuestion = 1;
	newQuestionNeeded = 0;
}

void loadAllChapters() {
	getQuestionsFromChapterFile("1.dat");
	getQuestionsFromChapterFile("2.dat");
	getQuestionsFromChapterFile("3.dat");
	getQuestionsFromChapterFile("4.dat");
	getQuestionsFromChapterFile("5.dat");
	getQuestionsFromChapterFile("6.dat");
	getQuestionsFromChapterFile("7.dat");
	getQuestionsFromChapterFile("8.dat");
	getQuestionsFromChapterFile("9.dat");
	getQuestionsFromChapterFile("10.dat");
	getQuestionsFromChapterFile("11.dat");
	getQuestionsFromChapterFile("12.dat");
	getQuestionsFromChapterFile("13.dat");
	getQuestionsFromChapterFile("14.dat");
	getQuestionsFromChapterFile("15.dat");
	getQuestionsFromChapterFile("16.dat");
	getQuestionsFromChapterFile("17.dat");
	getQuestionsFromChapterFile("18.dat");
	getQuestionsFromChapterFile("19.dat");
	getQuestionsFromChapterFile("20.dat");
	getQuestionsFromChapterFile("21.dat");
	getQuestionsFromChapterFile("22.dat");
}

void setColorScheme(int textcol, int bgcol, int buttoncol) {
	textcolor = textcol;
	bgcolor = bgcol;
	buttoncolor = buttoncol;
}

void changeGraphicsAndMusic() {
	if (key[KEY_F1]) {
		setColorScheme(RED, BLACK, GREEN);
	}
	else if (key[KEY_F2]) {
		setColorScheme(GREEN, BLACK, RED);
	}
	if (key[KEY_C]) 
	{
		destroy_midi(music);
		music = load_midi("houseofrisinsun.mid");
		play_midi(music, 0);
	}
	
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
	srand(time(NULL));
	buffer = create_bitmap(640, 480);
	buffers = create_bitmap(640, 480);
	set_mouse_sprite_focus(15, 15);
	show_mouse(screen);
	setColorScheme(RED, BLACK, GREEN);
	music = load_midi("houseofrisinsun.mid");
	SoundEffects[0] = load_sample("clapping.wav"); //winning answer
	SoundEffects[1] = load_sample("explode.wav"); //losing or dieing answer
	play_midi(music, 0);



	//create bitmaps
	//BITMAP* buffers = create_bitmap(SCREEN_W, SCREEN_H);
	//BITMAP* buffersAfter = create_bitmap(SCREEN_W, SCREEN_H);
	BITMAP* boneco = load_bitmap("boneco.bmp", NULL);
	BITMAP* sky = load_bitmap("sky.bmp", NULL);
	BITMAP* montanha = load_bitmap("montanha.bmp", NULL);
	BITMAP* dragon = load_bitmap("dragon.bmp", NULL);
	BITMAP* Vikings = load_bitmap("Vikings.bmp", NULL);
	BITMAP* scientist = load_bitmap("madScientist.bmp", NULL);
	BITMAP* space = load_bitmap("space.bmp", NULL);

	float teta = 0.0;






	// i do this so that the array has no zeroes in it (0 is the index of one of the questions that can be selected)
	//fillArrayWithNegatives(questionsDone, 220);
	while (!key[KEY_ESC]) 
	{
		clearScreen();
		updateGameInfo();
		changeGraphicsAndMusic();
		mousex = mouse_x;
		mousey = mouse_y;
		leftClick = (mouse_b & 1);

		
		//if in start screen mode, do start screen behavior
		if (startScreenMode == 1) 
		{
			//print title screen
			if (textcolor == RED) 
			{

				loadBackgroundImage("Introduction.bmp");
				
			}
			
			textout_centre_ex(buffer, font, "To turn off/on the music PRESS CTRL+M", SCREEN_W / 2, 22 + text_height(font), BLACK, WHITE);
			wrapText(buffer, font, "INSTRUCTIONS: Dodge your enemies and collect coins. Collect 50 points to win.", 15, SCREEN_H/2 + 150, BLACK, WHITE);
			textout_centre_ex(buffer, font, "Press ENTER to begin", SCREEN_W / 2,SCREEN_H/2 + 100,BLACK,WHITE);
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
				clearScreen();
			}

		}



		//else if in main menu mode, do main menu behavior
		if (mainMenuMode == 1) 
		{
			
			//update
			teta += 0.2;

			textout_ex(sky, font, "DodgeIT (ESC to quit)", 0, 1, BLACK, WHITE);
			

			//Note: Asking user to select their sprite from variety of the options available 
			textout_centre_ex(sky, font, "Select Your Player", SCREEN_W / 2, 22 + text_height(font), BLACK, WHITE);


			

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

					   

			//draw_sprite_ex(buffers, boneco, 250, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_H_FLIP);
			//draw_sprite_ex(buffers, boneco, 350, 330, DRAW_SPRITE_TRANS, DRAW_SPRITE_NO_FLIP);
			//rotate_sprite(buffers, boneco, 450, 330, ftofix(teta));
			//rotate_sprite_trans(buffers, boneco, 550, 330, ftofix(-teta));

			
			draw_sprite(screen, buffers, 0, 0);
			clear(buffers);
			
			

			//if selected F1: Boneco
			if (key[KEY_F1]) {

				mainMenuMode = 0;
				gamePlayMode = 1; 
				//clearScreen();
				clearScreens();


				//put the setting of the game
				//draw_sprite(buffers, space, 0, 0);
				//draw_sprite_ex(buffersF, boneco, 150, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);




				//play the game 


				//while playing the game show lifelines, score or any other information 

				// game is terminated by pressing ESC at any time show this 

				//game over stuff


				//draw_sprite(screen, buffers, 0, 0);
			}


			//if selected F2: Dragon
			else if (key[KEY_F2]) {
				mainMenuMode = 0;
				gamePlayMode = 2;
				//clearScreen();
			}


			//if selected F3: Viking
			else if (key[KEY_F3]) {
				mainMenuMode = 0;
				gamePlayMode = 3;
				//clearScreen();
			}


			//if selected F4: Scientist
			else if (key[KEY_F4]) {
				
				mainMenuMode = 0;
				gamePlayMode = 4;
				//clearScreen();
			}

			
		}



		if (gamePlayMode == 1) {

			set_trans_blender(225, 225, 255, 128);

					   

			draw_sprite(buffers, space, 0, 0);
			draw_sprite_ex(buffers, boneco, 150, 330, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);


			draw_sprite(screen, buffers, 0, 0);

		}



		/*
		else if (selectChapterMode == 1) {
			//get input for desired 
			if (keypressed()) {
				getInputFromKeyboard();
				if (key[KEY_ENTER]) {
					ustrzcpy(filename, 40, input);
					ustrzcat(filename, 16, ".dat");
					if (exists(filename)) {
						//load all questions
						getQuestionsFromChapterFile(filename);
						selectChapterMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input,10);
						inputIndex = 0;
					}
				}
			}
			if (!isInputDigit(input)) {
				textout_centre_ex(buffer, font, "Input must be a number.", SCREEN_W / 2, SCREEN_H / 2 + 100, textcolor, bgcolor);
			}
			textout_centre_ex(buffer, font, "Type in desired chapter number and press Enter to continue (1-22).", SCREEN_W / 2, SCREEN_H / 2 - 100, textcolor, bgcolor);
			textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, textcolor, bgcolor, "%s", input);
		}
		
		else if (selectUnitMode == 1) {
			//clear keyboard input
			if (keypressed()) {
				getInputFromKeyboard();
				if (key[KEY_ENTER]) {
					if (compareCharArrayToLiteral(input, '1')) {
						//Unit 1: Chapter 1, 2, and 3
						getQuestionsFromChapterFile("1.dat");
						getQuestionsFromChapterFile("2.dat");
						getQuestionsFromChapterFile("3.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '2')) {
						//Unit 2: Chapter 4, 5, and 6
						getQuestionsFromChapterFile("4.dat");
						getQuestionsFromChapterFile("5.dat");
						getQuestionsFromChapterFile("6.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '3')) {
						//Unit 3: Chapter 17
						getQuestionsFromChapterFile("17.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '4')) {
						//Unit 4: Chapters 7, 8, 9, and 10
						getQuestionsFromChapterFile("7.dat");
						getQuestionsFromChapterFile("8.dat");
						getQuestionsFromChapterFile("9.dat");
						getQuestionsFromChapterFile("10.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '5')) {
						//Unit 5: Chapter 11 
						getQuestionsFromChapterFile("11.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '6')) {
						//Unit 6: Chapters 12, 13, 14, 15, and 16
						getQuestionsFromChapterFile("12.dat");
						getQuestionsFromChapterFile("13.dat");
						getQuestionsFromChapterFile("14.dat");
						getQuestionsFromChapterFile("15.dat");
						getQuestionsFromChapterFile("16.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '7')) {
						//Unit 7: Chapters 18 and 19
						getQuestionsFromChapterFile("18.dat");
						getQuestionsFromChapterFile("19.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
					else if (compareCharArrayToLiteral(input, '8')) {
						//Unit 8: Chapters 20, 21, and 22
						getQuestionsFromChapterFile("20.dat");
						getQuestionsFromChapterFile("21.dat");
						getQuestionsFromChapterFile("22.dat");
						emptyCharArray(filename, 14);
						selectUnitMode = 0;
						selectNumberOfQuestionsMode = 1;
						emptyCharArray(input, 10);
						inputIndex = 0;
					}
				}
			}
			if (!isInputDigit(input)) {
				textout_centre_ex(buffer, font, "Input must be a number", SCREEN_W / 2, SCREEN_H / 2 + 100, textcolor, bgcolor);
			}
			textout_centre_ex(buffer, font, "Type in desired unit number and press Enter to continue (1-8).", SCREEN_W / 2, SCREEN_H / 2 - 100, textcolor, bgcolor);
			textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, textcolor, bgcolor, "%s", input);
		}

		else if (selectNumberOfQuestionsMode == 1) {
			
			if (keypressed()) {
				char *endpoint;
				getInputFromKeyboard();
				if (key[KEY_ENTER] && isInputDigit(input)) {
					numberOfQuestions = ustrtol(input, &endpoint, 10); 
					selectNumberOfQuestionsMode = 0;
					gameMode = 1;
					emptyCharArray(input, 10);
					inputIndex = 0;
				}
			}
			if (!isInputDigit(input)) {
				textout_centre_ex(buffer, font, "Input must be a number.", SCREEN_W / 2, SCREEN_H / 2 + 100, textcolor, bgcolor);
			}
			textout_centre_ex(buffer, font, "Type in desired number of questions (1-220) and press Enter to continue.", SCREEN_W / 2, SCREEN_H / 2 - 100, textcolor, bgcolor);
			textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, textcolor, bgcolor, "%s", input);
		}

		//else if in game mode, do game behavior
		else if (gameMode == 1) {
			//if user asked for more questions then there are, reduce it.
			if (numberOfQuestions > questionIndex) {
				numberOfQuestions = questionIndex;
			}
			if (!isFirstQuestion) {
				if (newQuestionNeeded) {
					newQuestionNeeded = 0;
					questionsDone[numberOfQuestions] = currentQuestion;
					while (isValueInArray(currentQuestion, questionsDone, 220)) {
						currentQuestion = rand() % questionIndex;
					}
				}
			}
			else {
				isFirstQuestion = 0;
				newQuestionNeeded = 0;
				currentQuestion = rand() % questionIndex;
			}
			//	display question text from questionarray[index]
			wrapText(buffer, font, questions[currentQuestion].questionText, 20, 20, textcolor, bgcolor);
			//	draw buttons for answers
			buttons[0] = drawNewButton(20, 90, 120, 190, buttoncolor, questions[currentQuestion].answers[0].buttonText, questions[currentQuestion].answers[0].isCorrectAnswer);
			buttons[1] = drawNewButton(20, 90, 210, 280, buttoncolor, questions[currentQuestion].answers[1].buttonText, questions[currentQuestion].answers[1].isCorrectAnswer);
			buttons[2] = drawNewButton(20, 90, 300, 370, buttoncolor, questions[currentQuestion].answers[2].buttonText, questions[currentQuestion].answers[2].isCorrectAnswer);
			buttons[3] = drawNewButton(20, 90, 390, 460, buttoncolor, questions[currentQuestion].answers[3].buttonText, questions[currentQuestion].answers[3].isCorrectAnswer);
			
			if (isButtonSelected(buttons[0]) && !leftClickBeingHeld) {
				if (buttons[0].isCorrectAnswer) {
					numberOfCorrectAnswers++;
					play_sample(SoundEffects[0],128,128,1000,FALSE);
				}
				else {
					numberOfIncorrectAnswers++;
					play_sample(SoundEffects[1], 128, 128, 1000, FALSE);
				}
				numberOfQuestions--;
				newQuestionNeeded = 1;
				leftClickBeingHeld = 1;
				if (numberOfQuestions <= 0) {
					//go to end screen
					gameMode = 0;
					endScreenMode = 1;
				}
			}
			if (isButtonSelected(buttons[1]) && !leftClickBeingHeld) {
				if (buttons[1].isCorrectAnswer) {
					numberOfCorrectAnswers++;
					play_sample(SoundEffects[0], 128, 128, 1000, FALSE);
				}
				else {
					numberOfIncorrectAnswers++;
					play_sample(SoundEffects[1], 128, 128, 1000, FALSE);
				}
				numberOfQuestions--;
				newQuestionNeeded = 1;
				leftClickBeingHeld = 1;
				if (numberOfQuestions <= 0) {
					//go to end screen
					gameMode = 0;
					endScreenMode = 1;
				}
			}
			if (isButtonSelected(buttons[2]) && !leftClickBeingHeld) {
				if (buttons[2].isCorrectAnswer) {
					numberOfCorrectAnswers++;
					play_sample(SoundEffects[0], 128, 128, 1000, FALSE);
				}
				else {
					numberOfIncorrectAnswers++;
					play_sample(SoundEffects[1], 128, 128, 1000, FALSE);
				}
				numberOfQuestions--;
				newQuestionNeeded = 1;
				leftClickBeingHeld = 1;
				if (numberOfQuestions <= 0) {
					//go to end screen
					gameMode = 0;
					endScreenMode = 1;
				}
			}
			if (isButtonSelected(buttons[3]) && !leftClickBeingHeld) {
				if (buttons[3].isCorrectAnswer) {
					numberOfCorrectAnswers++;
					play_sample(SoundEffects[0], 128, 128, 1000, FALSE);
				}
				else {
					numberOfIncorrectAnswers++;
					play_sample(SoundEffects[1], 128, 128, 1000, FALSE);
				}
				numberOfQuestions--;
				newQuestionNeeded = 1;
				leftClickBeingHeld = 1;
				if (numberOfQuestions <= 0) {
					//go to end screen
					gameMode = 0;
					endScreenMode = 1;
				}
			}
		}

		//need this in this one 
		else if (endScreenMode == 1) {
			textout_centre_ex(buffer, font, "Game Complete!!!", SCREEN_W/2, SCREEN_H/2 - 100, textcolor, bgcolor);
			textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, textcolor, bgcolor, "Correct Answers: %d Incorrect Answers: %d", numberOfCorrectAnswers, numberOfIncorrectAnswers);
			textout_centre_ex(buffer, font, "Press Ctrl to restart.", SCREEN_W / 2, SCREEN_H / 2 + 100, textcolor, bgcolor);
			if (key[KEY_LCONTROL] || key[KEY_RCONTROL]) {
				resetGame();
			}
		}
		*/

		/*
		blit(buffer, screen, 0, 0, 0, 0, 640, 480);
		
		if (leftClickBeingHeld && !leftClick) 
		{
			leftClickBeingHeld = 0;
		}
		rest(10);
		*/
		
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


	allegro_exit();
}
END_OF_MAIN()


