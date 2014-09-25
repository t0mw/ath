#ifndef ATH_H_INCLUDED
#define ATH_H_INCLUDED

#define ATH_UNUSED( arg ) (void)arg

#define ATH_CHECK( return_code ) do \
	{ \
		if( return_code < 0 ) \
		{ \
			printf( "Errcode: %d\n", return_code ); \
			abort(); \
		} \
	} \
	while(0)

typedef void ( *ath_fn )( void );
typedef int ath_id;

int ath_init( void );
ath_id ath_create( ath_fn main_fn, void *user_data, char *stack, int stack_size );
int ath_active_threads_count( void );
int ath_destroy( const ath_id id );
void ath_yield( void );
void ath_set_user_data( const ath_id id, void *user_data );
void ath_set_main_fn( const ath_id id, ath_fn main_fn );

#endif
