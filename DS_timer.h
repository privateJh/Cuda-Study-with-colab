//#pragma once
#ifndef _DS_TIMER_H
#define _DS_TIMER_H

#include <string> // std string
#include "DS_definitions.h"
#include <iostream>

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifdef _WIN32
	// For windows
	#include <Windows.h>
	typedef LARGE_INTEGER	TIME_VAL;
#else
	// For Unix/Linux
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/time.h>
	#include <string.h>	// c string
	typedef struct timeval	TIME_VAL;
#endif

#define TIMER_ON	true
#define TIMER_OFF	false

class DS_timer
{
private :

	bool turnOn	;

	UINT numTimer	;
	UINT numCounter ;

	// For timers
	bool*			timerStates ;
	TIME_VAL	ticksPerSecond;
	TIME_VAL	*start_ticks;
	TIME_VAL	*end_ticks;
	TIME_VAL	*totalTicks;

	char		timerTitle[255] ;
	std::string *timerName ;

	// For counters
	UINT *counters ;

	void memAllocCounters ( void ) ;
	void memAllocTimers ( void ) ;
	void releaseCounters ( void ) ;
	void releaseTimers ( void ) ;

public:
	DS_timer(int _numTimer = 1, int _numCount = 1, bool _trunOn = true );
	~DS_timer(void);

	// For configurations
	inline void timerOn ( void ) { turnOn = TIMER_ON ; }
	inline void timerOff ( void ) { turnOn = TIMER_OFF ; }

	UINT getNumTimer( void ) ;
	UINT getNumCounter ( void ) ;
	UINT setTimer ( UINT _numTimer ) ;
	UINT setCounter ( UINT _numCounter ) ;

	// For timers

	void initTimer(UINT id) ;
	void initTimers ( void );
	void onTimer(UINT id) ;
	void offTimer(UINT id ) ;
	double getTimer_ms(UINT id) ;

	void setTimerTitle ( char* _name ) { memset(timerTitle, 0, sizeof(char)*255) ; memcpy(timerTitle, _name, strlen(_name)) ; }

	void setTimerName (UINT id, std::string &_name) { timerName[id] = _name ; }
	void setTimerName (UINT id, char* _name) { timerName[id] = _name ;}

	// For counters

	void incCounter(UINT id) ;
	void initCounters( void ) ;
	void initCounter(UINT id) ;
	void add2Counter( UINT id, UINT num ) ;
	UINT getCounter ( UINT id ) ;

	// For reports

	void printTimer ( float _denominator = 1 ) ;
	void printToFile ( char* fileName, int _id = -1 ) ;
	void printTimerNameToFile ( char* fileName ) ;
} ;
//#ifndef _WIN32
//#define fprintf_s fprintf
//void fopen_s(FILE** fp, char* name, char* mode)
//{
//	*fp = fopen(name, mode);
//}
//#endif

/************************************************************************/
/* Constructor & Destructor                                             */
/************************************************************************/
DS_timer::DS_timer(int _numTimer /* =0 */, int _numCount /* =0 */, bool _trunOn /* =true */){

	turnOn = _trunOn ;
	start_ticks = NULL ; end_ticks = NULL ; totalTicks = NULL ;
	counters = NULL ;
	timerStates = NULL ;
	timerName = NULL ;

	numTimer = numCounter = 0 ;
	char name[255] = "DS_timer Report";
	setTimerTitle(name) ;

	setTimer(_numTimer ) ;
	setCounter( _numTimer ) ;

#ifdef _WIN32
	// For windows
	QueryPerformanceFrequency(&ticksPerSecond) ;
#endif
}

DS_timer::~DS_timer(void){
	releaseTimers() ;
	releaseCounters() ;
}

/************************************************************************/
/* Set & Get configurations                                             */
/************************************************************************/

// Memory allocation
void DS_timer::memAllocCounters( void ) {
	releaseCounters() ;
	counters = new UINT[numCounter] ;
	initCounters() ;
}

void DS_timer::memAllocTimers( void ) {
	releaseTimers() ;

	timerStates = new bool[numTimer] ;
	start_ticks = new TIME_VAL[numTimer];
	end_ticks = new TIME_VAL[numTimer];
	totalTicks = new TIME_VAL[numTimer];

	// Initialize
	memset(timerStates, 0, sizeof(bool)*numTimer);
	memset(start_ticks, 0, sizeof(TIME_VAL)*numTimer);
	memset(end_ticks, 0, sizeof(TIME_VAL)*numTimer);
	memset(totalTicks, 0, sizeof(TIME_VAL)*numTimer);

	timerName = new std::string[numTimer] ;
	for ( UINT i = 0 ; i < numTimer ; i++ ) {
		timerName[i].clear() ;
		timerName[i].resize(255) ;
	}
}

// Memory release
void DS_timer::releaseCounters( void ) { DS_MEM_DELETE_ARRAY(counters) ; }
void DS_timer::releaseTimers( void ) {
	//DS_MEM_DELETE(start_ticks) ; DS_MEM_DELETE(end_ticks) ; DS_MEM_DELETE(totalTicks) ;
	DS_MEM_DELETE_ARRAY(timerStates) ;
	DS_MEM_DELETE_ARRAY(start_ticks) ;
	DS_MEM_DELETE_ARRAY(end_ticks) ;
	DS_MEM_DELETE_ARRAY(totalTicks) ;
	DS_MEM_DELETE_ARRAY(timerName) ;
}

// Getters
UINT DS_timer::getNumTimer( void ) { return numTimer ; }
UINT DS_timer::getNumCounter( void ) { return numCounter ; }

// Setters
UINT DS_timer::setTimer( UINT _numTimer ) {
	if ( _numTimer == 0 )
		return 0 ;

	if (_numTimer <= numTimer)
		return numTimer;

	if ( numTimer != 0 ) {

		// Backup
		UINT oldNumTimer = numTimer ;
		TIME_VAL *oldTotalTicks = new TIME_VAL[oldNumTimer];
		memcpy(oldTotalTicks, totalTicks, sizeof(TIME_VAL)*oldNumTimer);

		numTimer = _numTimer ;
		memAllocTimers() ;

		memcpy(totalTicks, oldTotalTicks, sizeof(TIME_VAL)* oldNumTimer);
		delete oldTotalTicks ;
	} else {
		numTimer = _numTimer ;
		memAllocTimers() ;
	}

	return _numTimer ;
}

UINT DS_timer::setCounter( UINT _numCounter ) {

	if (_numCounter == 0 )
		return 0 ;

	if (_numCounter <= numCounter)
		return numCounter;

	if ( numCounter != 0 ) {

		// Backup
		int numOldCounter = numCounter ;
		UINT *oldCounters = new UINT[numOldCounter] ;
		memcpy(oldCounters, counters, sizeof(UINT)*numOldCounter) ;

		numCounter = _numCounter ;
		memAllocCounters() ;

		// Restore
		memcpy(counters, oldCounters, sizeof(UINT)*numOldCounter) ;
		delete oldCounters ;

	} else {
		numCounter = _numCounter ;
		memAllocCounters() ;
	}

	return numCounter ;

}

/************************************************************************/
/* Timer                                                                */
/************************************************************************/
void DS_timer::initTimer( UINT id ) {
	timerStates[id] = TIMER_OFF ;
#ifdef _WIN32
	totalTicks[id].QuadPart = 0 ;
#else
	totalTicks[id].tv_sec = 0;
	totalTicks[id].tv_usec = 0;
#endif
}

void DS_timer::initTimers( void ) {
	for ( UINT i = 0 ; i < numTimer ; i++ ) {
		initTimer(i);
	}
}

void DS_timer::onTimer( UINT id ) {
	if ( turnOn == false )
		return ;

	if ( timerStates[id] == TIMER_ON )
		return ;
#ifdef _WIN32
	QueryPerformanceCounter(&start_ticks[id]) ;
#else
	gettimeofday(&start_ticks[id], NULL);
#endif

	timerStates[id] = TIMER_ON ;
}

void DS_timer::offTimer( UINT id ) {
	if ( turnOn == false )
		return ;

	if ( timerStates[id] == TIMER_OFF )
		return ;

#ifdef _WIN32
	QueryPerformanceCounter(&end_ticks[id]) ;
	totalTicks[id].QuadPart = totalTicks[id].QuadPart + (end_ticks[id].QuadPart - start_ticks[id].QuadPart) ;
#else
	gettimeofday(&end_ticks[id], NULL);
	TIME_VAL period, previous;
	timersub(&end_ticks[id], &start_ticks[id], &period);
	previous = totalTicks[id];
	timeradd(&previous, &period, &totalTicks[id]);
#endif

	timerStates[id] = TIMER_OFF ;
}

double DS_timer::getTimer_ms( UINT id ) {
#ifdef _WIN32
	return ((double)totalTicks[id].QuadPart/(double)ticksPerSecond.QuadPart * 1000) ;
#else
	return (double)(totalTicks[id].tv_sec * 1000 + totalTicks[id].tv_usec / 1000.0);
#endif
}

/************************************************************************/
/* Counter                                                              */
/************************************************************************/
void DS_timer::incCounter( UINT id ) {
	if ( turnOn == false )
		return ;

	counters[id]++ ;
}

void DS_timer::initCounters( void ) {
	if ( turnOn == false )
		return ;

	for ( UINT i = 0 ; i < numCounter ; i++ )
		counters[i] = 0 ;
}

void DS_timer::initCounter( UINT id ) {
	if ( turnOn == false )
		return ;

	counters[id] = 0 ;
}

void DS_timer::add2Counter( UINT id, UINT num ) {
	if ( turnOn == false )
		return ;

	counters[id] += num ;
}

UINT DS_timer::getCounter( UINT id ) {
	if ( turnOn == false )
		return 0;

	return counters[id] ;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void DS_timer::printTimer( float _denominator){

	if ( turnOn == false )
		return ;

	//printf("\n*\t DS_timer Report \t*\n") ;
	printf("\n*\t %s \t*\n", timerTitle) ;
	printf("* The number of timer = %d, counter = %d\n", numTimer, numCounter ) ;
	printf("**** Timer report ****\n") ;

	for ( UINT i = 0 ; i < numTimer ; i++ ) {
		if ( getTimer_ms(i) == 0 )
			continue ;
		if ( timerName[i].c_str()[0] == 0 )
			printf("Timer %d : %.5f ms (%.5f ms)\n", i, getTimer_ms(i)/_denominator, getTimer_ms(i) ) ;
		else
			printf("%s : %.5f ms (%.5f ms)\n", timerName[i].c_str(), getTimer_ms(i)/_denominator, getTimer_ms(i) ) ;
	}

	printf("**** Counter report ****\n") ;
	for ( UINT i = 0 ; i < numCounter ;i++ ) {
		if ( counters[i] == 0 )
			continue ;
		printf("Counter %d : %.3f (%d) \n",i, counters[i]/_denominator, counters[i] ) ;
	}

	printf("*\t End of the report \t*\n") ;
}

void DS_timer::printToFile( char* fileName, int _id )
{
	if ( turnOn == false )
		return ;

	FILE *fp ;

	if ( fileName == NULL)
		fopen_s(&fp, "DS_timer_report.txt", "a") ;
	else {
		fopen_s(&fp, fileName, "a") ;
	}

	if ( _id >= 0 )
		fprintf_s(fp,"%d\t", _id) ;

	for ( UINT i = 0 ; i < numTimer ; i++ ) {
		if ( getTimer_ms(i) == 0 )
			continue ;
		fprintf_s(fp, "%s: %.9f\n", timerName[i].c_str(), getTimer_ms(i) ) ;
	}

	for ( UINT i = 0 ; i < numCounter ;i++ ) {
		if ( counters[i] == 0 )
			continue ;
		fprintf_s(fp, "%s: %d\n", timerName[i].c_str(), counters[i] ) ;
	}

	fprintf_s(fp, "\n") ;

	fclose(fp) ;
}

void DS_timer::printTimerNameToFile( char* fileName )
{
	if ( turnOn == false )
		return ;

	FILE *fp ;

	if ( fileName == NULL)
		fopen_s(&fp, "DS_timer_name.txt", "a") ;
	else {
		fopen_s(&fp, fileName, "a") ;
	}


	for ( UINT i = 0 ; i < numTimer ; i++ ) {
		if ( timerName[i].empty() )
			continue ;
		fprintf_s(fp, "%s\t", timerName[i].c_str() ) ;
	}
	fprintf_s(fp, "\n") ;
	fclose(fp) ;
}

#endif