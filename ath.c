#include "ath.h"
#include <ucontext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ATH_SCHED_QUEUE_SIZE 5
#define ATH_NO_ACTIVE_THREADS -1

struct ath_priv_context
{
	/* registers, stack, ... */
	ucontext_t ctx;
};

struct ath_obj
{
	ath_fn main_fn;
	int main_fn_returned;
	void *user_data;
	struct ath_priv_context thread_context;
};

static struct sched_data
{
	struct ath_obj threads_tab[ ATH_SCHED_QUEUE_SIZE ];
	int threads_tab_first_free;
	int active_thread;
	struct ath_priv_context main_thread_context;
} sched_data;

static void ath_priv_context_create( struct ath_priv_context *allocated_ctx, ath_fn main_fn, void *user_data, char *th_stack, int th_stack_size )
{
	getcontext( &allocated_ctx->ctx );
	allocated_ctx->ctx.uc_link = &sched_data.main_thread_context.ctx;
	allocated_ctx->ctx.uc_stack.ss_sp = th_stack;
	allocated_ctx->ctx.uc_stack.ss_size = th_stack_size;

	makecontext( &allocated_ctx->ctx, ( void (*)( void ) )main_fn, 1, (int)user_data );
}

static void ath_priv_context_save( struct ath_priv_context *allocated_ctx )
{
	getcontext( &allocated_ctx->ctx );
}

static void ath_priv_context_restore( const struct ath_priv_context *ctx )
{
	setcontext( &ctx->ctx );
}

int ath_init( void )
{
	sched_data.threads_tab_first_free = 0;
	sched_data.active_thread = ATH_NO_ACTIVE_THREADS;

	return 1;
}

ath_id ath_create( ath_fn main_fn, void *user_data, char *stack, int stack_size )
{
	int new_thread_id = sched_data.threads_tab_first_free;
	++sched_data.threads_tab_first_free;

	sched_data.threads_tab[ new_thread_id ].main_fn = main_fn;
	sched_data.threads_tab[ new_thread_id ].main_fn_returned = 0;
	sched_data.threads_tab[ new_thread_id ].user_data = user_data;

	ath_priv_context_create( &sched_data.threads_tab[ new_thread_id ].thread_context,
				 main_fn,
				 sched_data.threads_tab[ new_thread_id ].user_data,
				 stack,
				 stack_size );

	return new_thread_id;
}

int ath_active_threads_count( void )
{
	return sched_data.threads_tab_first_free;
}

int ath_destroy( const ath_id id )
{
	ATH_UNUSED( id );
	return 1;
}

static void ath_sched( void )
{
	// Round robin
	if( sched_data.active_thread == ATH_NO_ACTIVE_THREADS && sched_data.threads_tab_first_free >= 1 )
	{
		// Start new round
		printf( "[SCHED] start new round\n" );
		sched_data.active_thread = 0;
		ath_priv_context_restore( &sched_data.threads_tab[ 0 ].thread_context );
	}
	else if( sched_data.active_thread == sched_data.threads_tab_first_free - 1 || sched_data.threads_tab_first_free == 0 )
	{
		// Last running thread in queue, restore to main thread
		printf( "[SCHED] Last running thread in queue, restore to main thread\n" );
		sched_data.active_thread = ATH_NO_ACTIVE_THREADS;
		ath_priv_context_restore( &sched_data.main_thread_context );
	}
	else if( sched_data.threads_tab_first_free > sched_data.active_thread )
	{
		// Move to next thread
		++sched_data.active_thread;
		printf( "[SCHED] Move to next thread: %d\n", sched_data.active_thread );
		ath_priv_context_restore( &sched_data.threads_tab[ sched_data.active_thread ].thread_context );
	}
}

void ath_yield( void )
{
	if( sched_data.active_thread != ATH_NO_ACTIVE_THREADS )
	{
		ath_priv_context_save( &sched_data.threads_tab[ sched_data.active_thread ].thread_context );
	}
	else
	{
		ath_priv_context_save( &sched_data.main_thread_context );
	}
	
	// Swap context with scheduler's context?
	ath_sched();
}

void ath_set_user_data( const ath_id id, void *user_data )
{
	sched_data.threads_tab[ id ].user_data = user_data;
}

void ath_set_main_fn( const ath_id id, ath_fn main_fn )
{
	sched_data.threads_tab[ id ].main_fn = main_fn;
}
