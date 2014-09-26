#ifndef ATH_MUTEX_H_INCLUDED
#define ATH_MUTEX_H_INCLUDED

struct ath_mutex
{
	volatile int m;
};

void ath_mutex_init( struct ath_mutex *mtx );
void ath_mutex_lock( struct ath_mutex *mtx );
void ath_mutex_unlock( struct ath_mutex *mtx );
int ath_mutex_is_locked( struct ath_mutex *mtx );

#endif
