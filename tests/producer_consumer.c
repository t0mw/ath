#include "ath.h"
#include "ath_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREADS_STACK_SIZE 2048
static char producer_stack[ THREADS_STACK_SIZE ];
static char consumer_stack[ THREADS_STACK_SIZE ];

static char exch_buffer[ 1024 ] = { 0 };
static struct ath_mutex exch_buffer_mtx;
static int stop_flag = 0;

void producer( void )
{
	printf( "Producer started.\n" );
	static const char *text[] = { "Text no 1", "Text no 2", "Third text", "Lorem ipsum" };
	static const int text_size = sizeof( text ) / sizeof( *text );
	int counter = 0;
	while( !stop_flag )
	{
		sleep( 1 );
		ath_mutex_lock( &exch_buffer_mtx );
			printf( "Producing...\n" );
			ath_yield();
			strcpy( exch_buffer, text[ ( counter++ ) % text_size ] );
		ath_mutex_unlock( &exch_buffer_mtx );

		ath_yield();
	}
	printf( "Producer ended.\n" );
}

void consumer( void )
{
	printf( "Consumer started.\n" );
	while( !stop_flag )
	{
		sleep( 1 );
		ath_mutex_lock( &exch_buffer_mtx );
			printf( "Consuming...\n" );
			ath_yield();
			printf( "Exch buffer: %s\n", exch_buffer );
		ath_mutex_unlock( &exch_buffer_mtx );
		
		ath_yield();
	}
	printf( "Consumer ended.\n" );
}

int main( void )
{
	int rc = 0;
	
	rc = ath_init();
	ATH_CHECK( rc );
	
	rc = ath_create( producer, NULL, producer_stack, THREADS_STACK_SIZE );
	ATH_CHECK( rc );
	
	rc = ath_create( consumer, NULL, consumer_stack, THREADS_STACK_SIZE );
	ATH_CHECK( rc );
	
	ath_mutex_init( &exch_buffer_mtx );

	printf( "Starting loop\n" );
	
	int counter = 0;
	for( counter = 0; counter < 10; ++counter )
	{
		ath_yield();
	}
	
	stop_flag = 1;
	while( ath_active_threads_count() != 1 )
	{
		ath_yield();
	}
	
	return 0;
}