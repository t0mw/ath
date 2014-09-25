#include "ath.h"
#include <ucontext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ATH_SCHED_QUEUE_SIZE 5
#define ATH_SCHED_STACK_SIZE 2048
#define ATH_NO_ACTIVE_THREADS -1

struct ath_priv_context
{
	/* registers, stack, ... */
	ucontext_t ctx;
};

struct ath_obj
{
	ath_fn main_fn;
	int finished;
	void *user_data;
	struct ath_priv_context thread_context;
};

static struct sched_data
{
	struct ath_obj threads_tab[ ATH_SCHED_QUEUE_SIZE ];
	int threads_tab_first_free;
	int active_threads_count;
	int active_thread;

	char scheduler_stack[ ATH_SCHED_STACK_SIZE ];
	struct ath_priv_context scheduler_context;
} sched_data;

// static void ath_priv_context_swap( struct ath_priv_context *allocated_ctx_to_save, struct ath_priv_context *allocated_ctx_to_restore )
// {
//	/* todo */
// }

// static void ath_priv_context_create()
// {
//	/* todo */
// }

static void ath_sched( void )
{
	// Round robin
	while(1)
	{
		// printf( "[SCHED] start, active thread: %d , first free: %d ", sched_data.active_thread, sched_data.threads_tab_first_free );
		sched_data.active_thread = ( sched_data.active_thread + 1 ) % sched_data.threads_tab_first_free;
		if( sched_data.threads_tab[ sched_data.active_thread ].finished )
		{
			// printf( "finished, trying next\n" );
			continue;
		}
		// printf( "next: %d\n", sched_data.active_thread );
		swapcontext( &sched_data.scheduler_context.ctx, &sched_data.threads_tab[ sched_data.active_thread ].thread_context.ctx );
	}
}

int ath_init( void )
{
	sched_data.threads_tab_first_free = 1;
	sched_data.active_threads_count = 1;
	sched_data.active_thread = 0;

	getcontext( &sched_data.scheduler_context.ctx );
	sched_data.scheduler_context.ctx.uc_link = NULL;
	sched_data.scheduler_context.ctx.uc_stack.ss_sp = sched_data.scheduler_stack;
	sched_data.scheduler_context.ctx.uc_stack.ss_size = ATH_SCHED_STACK_SIZE;
	makecontext( &sched_data.scheduler_context.ctx, ( void (*)( void ) )ath_sched, 0 );

	return 1;
}

static void ath_thread_entry_point( void )
{
	sched_data.threads_tab[ sched_data.active_thread ].main_fn();
	sched_data.threads_tab[ sched_data.active_thread ].finished = 1;
	--sched_data.active_threads_count;
}

ath_id ath_create( ath_fn main_fn, void *user_data, char *stack, int stack_size )
{
	int new_thread_id = sched_data.threads_tab_first_free;
	++sched_data.threads_tab_first_free;
	
	++sched_data.active_threads_count;

	getcontext( &sched_data.threads_tab[ new_thread_id ].thread_context.ctx );
	sched_data.threads_tab[ new_thread_id ].thread_context.ctx.uc_link = &sched_data.scheduler_context.ctx;
	sched_data.threads_tab[ new_thread_id ].thread_context.ctx.uc_stack.ss_sp = stack;
	sched_data.threads_tab[ new_thread_id ].thread_context.ctx.uc_stack.ss_size = stack_size;
	
	sched_data.threads_tab[ new_thread_id ].finished = 0;
	sched_data.threads_tab[ new_thread_id ].main_fn = main_fn;
	
	sched_data.threads_tab[ new_thread_id ].user_data = user_data;

	makecontext( &sched_data.threads_tab[ new_thread_id ].thread_context.ctx, ( void (*)( void ) )ath_thread_entry_point, 0 );

	return new_thread_id;
}

int ath_active_threads_count( void )
{
	return sched_data.active_threads_count;
}

int ath_destroy( const ath_id id )
{
	sched_data.threads_tab[ id ].finished = 1;
	return 1;
}

void ath_yield( void )
{
	// printf( "[YIELD] creating thread context (active: %d)\n", sched_data.active_thread );
	swapcontext( &sched_data.threads_tab[ sched_data.active_thread ].thread_context.ctx, &sched_data.scheduler_context.ctx );
	// printf( "[YIELD] returned here (active: %d)\n", sched_data.active_thread );
}

void ath_set_user_data( const ath_id id, void *user_data )
{
	sched_data.threads_tab[ id ].user_data = user_data;
}

void *ath_get_user_data( void )
{
	return sched_data.threads_tab[ sched_data.active_thread ].user_data;
}

void ath_set_main_fn( const ath_id id, ath_fn main_fn )
{
	sched_data.threads_tab[ id ].main_fn = main_fn;
}
