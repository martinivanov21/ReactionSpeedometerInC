#include <stdlib.h>
#include <util/delay.h>
#include <ledlight.h>
#include <button.h>
#include <usart.h>
#include <avr/interrupt.h>
#include <display.h>
#include <buzzer.h>
#include <avr/io.h>
#include <poten.h>

typedef struct {
    int responseTime;
    int levelNr;
} LevelData;

typedef struct {
    LevelData levelData[10];
} ALL_LEVELS;

// the heap memory, but I do not have enough space :(
// typedef struct {
//     LevelData *levelData;
// } ALL_LEVELS;

ALL_LEVELS allLevels;
uint8_t randomSeed;
volatile uint32_t timerCount = 0;
volatile uint8_t reactionFlag = 0;
char *charss[] = {"L","C","R"};
uint32_t reactionTime;

void initTimer() {
    TCCR1B |= _BV( WGM12);
    OCR1A = 15624; // 1 second interval  for 1024
    TIMSK1 |= _BV( OCIE1A); // Enable the interrupt
    TCCR1B |= (_BV( CS12) | ( CS10)); 
}
ISR(TIMER1_COMPA_vect) {
    timerCount++;
}

ISR(PCINT1_vect) {
    if (buttonPushed(1)) {
        reactionFlag = 2;
    }
    if (buttonPushed(0))
    {
      reactionFlag=1;
    }
    if (buttonPushed(2))
    {
      reactionFlag =3;
    }
}


int chooseLevel(void);
void level_1(void);
void level_2(void);
void level_3(void);
void level_4(void);
void level_5(void);
int continueGame(void);
void calculateAge(void);
void displayLevelAndRound( int round);
int chooseTone();


int main(void)
{
  initDisplay();
  initTimer();
  initUSART();
  initADC();
  

  enableButton(0);
  enableButton(1);
  enableButton(2);
  enableAllButtonInterrupts();

  printf("\n\n\n=============Reaction Speedometer=============");
  printf("\nThis game will measure your reaction time.");
  printf("\nIt has 5 different levels which will measure your skills.");
  printf("\nTo choose a level: Press \"Button 1\" to increase the level, \"Button 3\" to decrease and \"Button 2\" to confirm.");
  printf("\nEach level has 10 different series of tests.");

  sei();

  randomSeed = timerCount;
  srand(randomSeed);

  //for the malloc memory but I do not have enough space :(
  // allLevels.levelData = (LevelData *)malloc(10 * sizeof(LevelData));
  
  while (1)
    {
      //chose the level
      int choosenLevel= chooseLevel();
    
      switch (choosenLevel)
    {
      case 1: level_1();
      break;
      case 2: level_2();
      break;
      case 3: level_3();
      break;
      case 4: level_4();
      break;
      case 5: level_5();
      break;
    }

    int continueChoice = continueGame();
    if (continueChoice==0)
    {
      break;
    }
    


 }
  free(allLevels.levelData);
    
  return 0;
}

int chooseLevel(void){
    int currentLevel = 1;
    
    while (1)
    {
      if (reactionFlag==1)
      {
        reactionFlag = 0;
        if (currentLevel < 5)
        {
          currentLevel++;
        }
        
      }
      if (reactionFlag==3)
      {
        reactionFlag=0;
        if (currentLevel>1)
        {
          currentLevel--;
        }
      }

      if (reactionFlag==2)
      {
        reactionFlag = 0;
        return currentLevel;
      }
       // Displaying the current level
        writeCharToSegment(0, 'l');
        _delay_ms(5);
        writeCharToSegment(1, 'V');
        _delay_ms(5);
        writeCharToSegment(2, 'L');
        _delay_ms(5);
        writeNumberToSegment(3, currentLevel);
        _delay_ms(5);
      
    }
}

void level_1(void)
{
  printf("\n\n\n===========You choose Level 1===========");
  printf("\nOne of the 3 dots will light up(1, 2 or 3) and you have to click the corresponding button - from left to right (1, 2, 3)");
  printf("\nBE QUCIK!");
  for (int i = 0; i < 10; i++)
  {

    clearDisplay();
    _delay_ms(1000);


    displayLevelAndRound(i+1);
    clearDisplay();
    srand(timerCount+i+rand());
    int dotLight = rand()%3;
    writeDot(dotLight);

    timerCount=0;
    reactionFlag=0;
    while (reactionFlag != (dotLight+1));
    reactionTime = timerCount*100;
    clearDisplay();
    printf("\nLevel 1 - Round %d: %ld ms\n",i+1,reactionTime);

    allLevels.levelData[i].responseTime=reactionTime;
    allLevels.levelData[i].levelNr = i+1;
    _delay_ms(1000);
    
  }
  
}

void level_2(void)
{
  printf("\n\n\n===========You choose Level 2===========");
  printf("\nThe display will show the letters: \"L\", \"C\" and \"R\". You have to press the corresponding button - from left to right (\"L\", \"C\" and \"R\")");
  // initDisplay();
  for (int i = 0; i < 10; i++)
  {
      displayLevelAndRound(i+1);
      clearDisplay();
      srand(timerCount+i+rand());
      int randomChar = rand()%3;
      writeCharToSegment(0,charss[randomChar][0]);

      timerCount=0;
      reactionFlag=0;
      while (reactionFlag != (randomChar+1))
      {

      }
      reactionTime = timerCount * 100;
      clearDisplay();
      printf("\nLevel 2 - Round %d: %ld ms\n",i+1,reactionTime);     
      allLevels.levelData[i].responseTime=reactionTime;
      allLevels.levelData[i].levelNr = i+1;
      _delay_ms(1000); 
        

  }
  

}

void level_3(void){
    printf("\n\n\n===========You choose Level 3===========");
    printf("\nThe screen will blink 1, 2 or 3 times and you have to click the corresponding button - from left to right (1, 2, 3)");
    clearDisplay();
    
    
    for (int i = 0; i < 10; i++) {
      
        displayLevelAndRound(i+1);
        clearDisplay();
        
      
     srand(timerCount+i+rand());
    int blinkTimes = rand() % 3 + 1;
    // Randomly choose 1, 2, or 3 blinks

    for (int j = 0; j < blinkTimes; j++) {
       reactionFlag = 0;
        enableAllLeds();
        lightUpAllLeds();
        _delay_ms(500);
        lightDownAllLeds();
        _delay_ms(500);
  
    }

    timerCount = 0;
    reactionFlag = 0;

    while (1) {
        if (reactionFlag ==1 || reactionFlag == 2 || reactionFlag ==3)
        {
          break;
        }
        
    }

    if (reactionFlag == blinkTimes)
    {
    reactionTime = timerCount * 100; 
    clearDisplay();
    printf("\nLevel 3 - Round %d: %ld ms\n", i + 1, reactionTime);     
    allLevels.levelData[i].responseTime = reactionTime;
    allLevels.levelData[i].levelNr = i + 1;
    }
    _delay_ms(1000); 
    reactionFlag =0;
  }
}


void level_4()
{
  printf("\n\n\n===========You choose Level 4===========");
  printf("\nThe buzzer will buzz different times (1, 2 or 3) and you have to click the corresponding button - from left to right (1, 2, 3)");
   printf("\nChoose with what tone your buzzer will ring. You have to rotate the potentiometer to choose.");
  printf("\nFrom 0 to 340 is C5, from 341 to 679 is F5 and for 680 and up is C6.");
  printf("\nPress button 1 to confirm.");

  int chosenValueForBuzzer = chooseTone();

  for (int i = 0; i < 10; i++) {
      
      displayLevelAndRound(i+1);
      clearDisplay();
      srand(timerCount+i+rand());
    
    int buzzTime = rand()%3;

    
    reactionFlag=0;
    for (int k = 0; k<buzzTime+1;k++)
  {

    enableBuzzer();
    _delay_ms(750);
    selectToneADC(chosenValueForBuzzer);
    _delay_ms(500);
  }
  timerCount=0;
  while (reactionFlag != (buzzTime+1));
  reactionTime=timerCount*100;
  clearDisplay();
  printf("\nLevel 4 - Round %d: %ld ms",i+1,reactionTime);
  allLevels.levelData[i].responseTime=reactionTime;
  allLevels.levelData[i].levelNr = i+1;
  _delay_ms(1000); 

  }
}
void level_5()
{
  printf("\n\n\n===========You choose Level 5===========");
  printf("\nThe display will show the letters: \"L\", \"C\" and \"R\". You have to press the corresponding button - from left to right (\"L\", \"C\" and \"R\")");
  printf("\nBe mindful, there might be signs that confuse you.");
  printf("\nOnly click on the buttons of the matching letters!");
    clearDisplay();
  
    for (int i = 0; i < 10; i++)
    {

      displayLevelAndRound(i+1);
      clearDisplay();
      srand(timerCount+i+rand());
      int distraction = rand()%2;// to decide if will add a distraction

      if (distraction)
      {
        int distractionType = rand()%2; //0 is for the buzzer and 1 for the light;
        if (distractionType ==0)
        {
          enableBuzzer();
          playTone(C5,500);
          _delay_ms(500);
        }else        
        {
          enableAllLeds();
          lightUpAllLeds();
          _delay_ms(500);
          lightDownAllLeds();
    
        }
        
      }
      srand(timerCount+i+rand());
      int randomChar = rand()%3;
      writeCharToSegment(randomChar,charss[randomChar][0]);

      timerCount=0;
      reactionFlag=0;

      while (reactionFlag!=(randomChar+1))
      {
        
      }
      reactionTime=timerCount*100;
      clearDisplay();
      printf("\nLevel 5 - Round %d: %ld ms\n",i+1, reactionTime);
      allLevels.levelData[i].responseTime = reactionTime;
      allLevels.levelData[i].levelNr = i + 1;
      _delay_ms(1000);


      
      
    }
    
    
}



int continueGame(){

  clearDisplay();
  printf("\n\n\nDo you want to continue the game?");
  printf("\nClick button 1 if you want to play again.\nClick button 2 if you want to see your stats.\n\n\n");
   while (1)
    {
      reactionFlag=0;
        writeString("CNT");
        writeQuestionmark(3);
        _delay_ms(5);

        if (reactionFlag==1)
        {
          reactionFlag = 0;
          return 1; // this will go back to the loop for the level
        }
        if (reactionFlag==2)
        {
          reactionFlag = 0;
          calculateAge(); // this will display the statastics about the age
          return 0;
        }
        
        
    }
  
  }

  void printAge(int age) {
    writeNumberToSegment(0, age / 10);
    _delay_ms(5);
    writeNumberToSegment(1, age % 10);
    _delay_ms(5);
    writeCharToSegment(2, 'Y');
    _delay_ms(5);
    writeCharToSegment(3, 'R');
    _delay_ms(5);
}

  void calculateAge(void) {
    int totalReactionTime = 0;
    for (int i = 0; i < 10; i++) {
        totalReactionTime += allLevels.levelData[i].responseTime;
    }
    printf("\nTotal Reaction Time: %d ms\n", totalReactionTime);

    int averageTime = totalReactionTime / 10;

    if (averageTime < 310) {
        printf("\nYour reaction time tells me that you are: %d years old or below\n", 20);
        while (1) {
            printAge(20);
        }
    } else if (averageTime <= 800) {
        int age = (averageTime - 301) / 45 + 25;
        printf("\nYour reaction time tells me that you are: %d years old\n", age);
        while (1) {
            printAge(age);
        }
    } else {
        printf("\nYour reaction time tells me that you are 80 years old or older\n");
        while (1) {
            printAge(80);
        }
    }
}


void displayLevelAndRound( int round) {
    writeNumberAndWait(round,750);
}

int chooseTone(){
  initADC();
  ADCSRA |= ( 1 << ADSC );    
  loop_until_bit_is_clear( ADCSRA, ADSC );   
  uint16_t value = ADC; 
  while (!buttonPushed(0))
  {
  ADCSRA |= (1 << ADSC);    
  loop_until_bit_is_clear(ADCSRA, ADSC);
   value = ADC; 
        
   writeNumber(value);
   blankSegment(3);
   _delay_ms(5);  
  }
  return value;
  
  
}