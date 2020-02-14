/*****************************************/
/* PROJECT: Stairs LEDS               	 */
/* DATE: 2018.11 -    		          	 */
/* CHECKED: 2019.03.22	              	 */
/* UPDATED: 2020.02.13	              	 */
/*****************************************/
#include <REGX51.H>

/***  MACROS ***/
#define _LED P1_0
#define _ALL_LIGHTS_OFF 0x0000
#define _LIGHT_DETECTOR P1_1
#define _DOWN_DETECTOR P3_2
#define _UP_DETECTOR P3_3
#define _ALL_LIGHTS_STATUS P1_7
#define _TIMER_16BIT_MODE 0x01
#define _TIMER_ENABLE	1
#define _IE_ENABLE_TIMER	0x82
#define _ID_DISABLE_ALL	0x00
#define _STATES_DEFAULT 0x0000

/*
*	16 - delay between timer interrupt when person is detected (default 16)
*	Changed to 14 , 22.03.2019
* Changed to 10 , 11.07.2019
* Changed to  8 , 24.11.2019
* changed to  3 , 17.12.2019
*/
#define _TIMER_DELAY_MOVE_DETECTOR	3

/*
*	10 - ~10.49 sec to go through 16 steps -> ~0.6 sec per step
*	Changed from 10 to 9, 22.03.2019
* Changed from 9 to 7, 24.11.2019
* Changed from 9 to 6, 17.12.2019
* Changed from 6 to 8, 13.02.2020
*/
#define _TIMER_DELAY_LIGHTS_MOVEMENT	8


/** New consts from 11.07.2019  **/
// Delay for preventing moving sensors to re-detect  person when he finished movement
//name changed 2020.02.13
//Default value: 10
#define _REDETECT_PREVENT_DELAY 10
//Led blink delay for idle (night) mode
#define _IDLE_DELAY 	80
//Led blink delay for daily (light detected) mode
#define _DAILY_DELAY 	150

/***  ****** ***/

/***  GLOBAL VARIABLES ***/
code unsigned int BLOCKER_STATE_LEDS_DOWN = 0x003F;	//0000 0000  0011 1111
code unsigned int BLOCKER_STATE_LEDS_UP = 0xFC00;	//1111 1100  0000 0000
code unsigned int STARTING_STATE_DOWN	= 0x001F;	//0000 0000  0001 1111
code unsigned int STARTING_STATE_UP	= 0xF800;	//1111 1000  0000 0000

code unsigned char table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };


volatile bit lastDownSensorState;
volatile bit lastUpSensorState;
volatile bit personDownDetected;
volatile bit personUpDetected;



volatile unsigned int currentLedState;

volatile unsigned int stateFromUpToDown;	//state from TOP to DOWN
volatile unsigned int stateFromDownToUp;	//state from DOWN to UP

volatile unsigned char timerLightsMovementCounter;	//timer - how many times timer ticks before moving lights up/down
volatile unsigned char timerMoveDetectorCounter;	//timer - how many times timer ticks before checking movement detector state
/***  **************** ***/

volatile unsigned char ledDelay; //delay for bliniking led

volatile unsigned char downReDetectPreventCounter;	//counter - how many times to skip before checking down movement detector state
volatile unsigned char upReDetectPreventCounter;	//counter - how many times to skip before checking up movement detector state


/*****************************************/
/***************** HELPERS ***************/
/*****************************************/
unsigned char Reverse(unsigned char number)
{
	return table[number];
}

void SetLeds(int state)
{
	P0 = Reverse(state >> 8);
	P2 = state & 0x00FF;
}


void Delay(int time)
{
	unsigned int d1;
	unsigned int d2 ;
	for(d1 = 0; d1 < time; d1++)
	{
		for(d2 = 0; d2 < 2000; d2++)
		{
			;
		}
	}
}

/*CHECKING SENSORS*/
void CheckDownSensorState(void)
{
	bit currentState = _DOWN_DETECTOR;
	
	if(downReDetectPreventCounter > 0)
	{
		downReDetectPreventCounter--;
		personDownDetected = 0;
		return;
	}

	if(lastDownSensorState && currentState)
	{
		personDownDetected = 1;
		lastDownSensorState = 0;
	}
	else
	{
		personDownDetected = 0;
		lastDownSensorState = currentState;
	}
}
void CheckUpSensorState(void)
{
	bit currentState = _UP_DETECTOR;
	
	if(upReDetectPreventCounter > 0)
	{
		upReDetectPreventCounter--;
		personUpDetected = 0;
		return;
	}

	if(lastUpSensorState && currentState)
	{
		personUpDetected = 1;
		lastUpSensorState = 0;
	}
	else
	{
		personUpDetected = 0;
		lastUpSensorState= currentState;
	}
}





bit CanStartLedsFromDown()
{
	//peson detected and 6 leds from down are turned off (0000 0000 0011 1111)
	return personDownDetected & !((stateFromDownToUp | stateFromUpToDown) & BLOCKER_STATE_LEDS_DOWN);
}
bit CanStartLedsFromUp()
{
	return personUpDetected & !((stateFromUpToDown | stateFromDownToUp) & BLOCKER_STATE_LEDS_UP);
}




void SetDownStartingLeds()
{
	stateFromDownToUp = stateFromDownToUp | STARTING_STATE_DOWN;
}
void SetUpStartingLeds()
{
	stateFromUpToDown = stateFromUpToDown | STARTING_STATE_UP;
}

void DecrementTimerCounters(void)
{
	timerLightsMovementCounter--;
	timerMoveDetectorCounter--;
}


void MoveLeds(void)
{
	//if state from down to up is to be cleared on the top
	//pin 16: 1 -> 0, but all other 5 pins are already 0
	//then set the delay for sensors
	if((stateFromDownToUp & BLOCKER_STATE_LEDS_UP) == 0x8000)
	{
		upReDetectPreventCounter = _REDETECT_PREVENT_DELAY;
	}


	//if state from up to down is to be cleared on the bottom
	//pin 1: 1 -> 0, but all other 5 pins are already 0
	//then set the delay for sensors
	if((stateFromUpToDown & BLOCKER_STATE_LEDS_DOWN) == 0x0001)
	{
		downReDetectPreventCounter = _REDETECT_PREVENT_DELAY;
	}

	
	stateFromDownToUp = stateFromDownToUp << 1;
	stateFromUpToDown = stateFromUpToDown >> 1;
}

void SetCurrentState(void)
{
currentLedState = (stateFromDownToUp | stateFromUpToDown);
}

void SetDayMode(void)
{
	//turn leds off
	SetLeds(_ALL_LIGHTS_OFF);
	//reset all timer counters
	timerMoveDetectorCounter = _TIMER_DELAY_MOVE_DETECTOR;
	timerLightsMovementCounter = _TIMER_DELAY_LIGHTS_MOVEMENT;
	//reset all states
	stateFromDownToUp = _STATES_DEFAULT;
	stateFromUpToDown = _STATES_DEFAULT;
	currentLedState = _STATES_DEFAULT;
	personDownDetected = 0;
	personUpDetected = 0;
	lastDownSensorState	= 0;
	lastUpSensorState = 0;
}


/*****************************************/
/*****************************************/
void InitVariables(void)
{
	timerLightsMovementCounter = _TIMER_DELAY_LIGHTS_MOVEMENT;
	timerMoveDetectorCounter = _TIMER_DELAY_MOVE_DETECTOR;

	stateFromUpToDown = 1;
	stateFromDownToUp = 0;

	lastDownSensorState = 0;
	lastUpSensorState = 0;
	personDownDetected = 0;
	personUpDetected = 0;
	currentLedState = 0x0000;
}

void InitTimerInterrupt(void)
{
	//set timer values
	TMOD = _TIMER_16BIT_MODE;
	//enable timer T0
	TR0  = _TIMER_ENABLE;
	//enable timer interrupts
	IE   = _IE_ENABLE_TIMER;
}

/*****************************************/
/************ INTERRUPT ******************/
/*****************************************/
void timer_interrupt(void) interrupt 1
{
	// 1 - light detected
	// 0 - night mode
	if(_LIGHT_DETECTOR)
	{
		SetDayMode();
		//go out
		return;
	}

	DecrementTimerCounters();

	if(!timerLightsMovementCounter) //move lights
	{
		MoveLeds();
		timerLightsMovementCounter = _TIMER_DELAY_LIGHTS_MOVEMENT;
	}

	if(!timerMoveDetectorCounter) //check detectors states
	{
		//check states of move detectors
		CheckDownSensorState();
		CheckUpSensorState();

		if(CanStartLedsFromDown())
		{
			SetDownStartingLeds();
		}
		if(CanStartLedsFromUp())
		{
			SetUpStartingLeds();
		}

		//RESET timer counter
		timerMoveDetectorCounter = _TIMER_DELAY_MOVE_DETECTOR;
	}

	SetCurrentState();
	SetLeds(currentLedState);
	
	// BLINK P1_7
	if(currentLedState)
	{
			_ALL_LIGHTS_STATUS = 1;
	}
	else 
	{
			_ALL_LIGHTS_STATUS = 0;
	}
}
/*****************************************/
/*****************************************/


/*****************************************/
/*****************************************/
/***************** MAIN ******************/
/*****************************************/
/*****************************************/
void Main(void)
{
	unsigned char i;
	//unsigned char j;

	Delay(250); //delay on start
	Delay(250); //delay on start	
	Delay(250); //delay on start
	Delay(250); //delay on start

	P1_1 = 1; // - input on pin P1.1
	_DOWN_DETECTOR = 1;
	_UP_DETECTOR = 1;
	
	P0 = 0xFF;
	P2 = 0xFF;
	//P3_2 = 1; //input on all pins
	//P3_3 = 1;
	//P3_2 = 0; 
	//P3_3 = 0;
	_LED = 0;

	for(i = 0 ; i < 16; i++)
	{
		SetLeds(0x0001 << i);
		Delay(50);
		_LED = !_LED;
	}
	
	
	SetLeds(_ALL_LIGHTS_OFF);
	Delay(220);

	//triple blink
	_LED = !_LED;
	SetLeds(0xAAAA);
	Delay(220);

	_LED = !_LED;
	SetLeds(0x5554);
	Delay(220);
	
	_LED = !_LED;
	SetLeds(0xAAAA);
	Delay(220);

	SetLeds(_ALL_LIGHTS_OFF);
	_LED = !_LED;
	Delay(800);


	InitVariables();
	InitTimerInterrupt();

	while(1)
	{
		_LED = !_LED;
		if(_LIGHT_DETECTOR)
		{
				Delay(_DAILY_DELAY);
		}
		else Delay(_IDLE_DELAY);
	}

}




















//TODO: in interrupt: what should be first: moving leds or checking detectors? (I believe checking detectors)
//TODO: how to set leds and then not move them (situation where from 0x0000 there is 0x001F and in the same interrupt it goes up to 0x003E)
//		- maybe firstly move leds and then setting state from detectors?)

/*
interrupt:
	IF day-light-detector is set
		RESET all leds
		RESET timer-counters
		RESET all states
		RETURN


	DECREMENT timer-counter-for-moving-leds;
	DECREMENT timer-counter-for-checking-detectors;
	
	IF time-to-move-leds
		shift leds (MoveLeds())
		SET next led states (SetCurrentState())
		RESET timer-counter-for-moving-leds

	IF time-to-check-detectors
		check-down-detector-state
		check-up-detector-state
		IF person-down-detected (CanStartLedsFromDown())
			SET starting-down-state (SetDownStartingLeds())
		IF person-up-detected (CanStartLedsFromUp())
			SET starting-un-state (todo: function())

	SET lights (SetLeds())
*/













