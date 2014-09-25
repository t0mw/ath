#include "../ath.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TMP_STACK_SIZE 2048

static char thread_one_stack[ TMP_STACK_SIZE ];
static char thread_two_stack[ TMP_STACK_SIZE ];

static volatile int flag_one = 0;
static volatile int flag_two = 0;

void thread_one( void )
{
	printf( "%s started\n", __PRETTY_FUNCTION__ );
	while( !flag_one )
	{
		printf( "%s yielding proc\n", __PRETTY_FUNCTION__ );
		sleep(2);
		ath_yield();
	}
	printf( "%s ended\n", __PRETTY_FUNCTION__ );
}

void thread_two( void )
{
	printf( "%s started\n", __PRETTY_FUNCTION__ );
	while( !flag_two )
	{
		printf( "%s yielding proc\n", __PRETTY_FUNCTION__ );
		sleep(2);
		ath_yield();
	}
	printf( "%s ended\n", __PRETTY_FUNCTION__ );
}

int main( void )
{
	int rc = 0;

	rc = ath_init();
	ATH_CHECK( rc );

	ath_id th_one_id = ath_create( thread_one, NULL, thread_one_stack, TMP_STACK_SIZE );
	ATH_CHECK( th_one_id );
	printf( "Thread one ID: %d\n", th_one_id );

	ath_id th_two_id = ath_create( thread_two, NULL, thread_two_stack, TMP_STACK_SIZE );
	ATH_CHECK( th_two_id );
	printf( "Thread two ID: %d\n", th_two_id );
	
	int scheds_counter = 0;
	while( 1 )
	{
		int active_threads = ath_active_threads_count();
		printf( "Currently active threads: %d\n", active_threads );
		if( active_threads == 1 )
		{
			break;
		}

		ath_yield();

		++scheds_counter;
		if( scheds_counter == 1 )
		{
			printf( "Setting flag one\n" );
			flag_one = 1;
		}
		else if( scheds_counter == 2 )
		{
			printf( "Setting flag two\n" );
			flag_two = 1;
		}
	}

	return 0;
}
