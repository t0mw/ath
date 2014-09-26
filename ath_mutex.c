#include "ath_mutex.h"
#include "ath.h"
#include <stdio.h>

void ath_mutex_init( struct ath_mutex *mtx )
{
	mtx->m = 0;
}

void ath_mutex_lock( struct ath_mutex *mtx )
{
	while( mtx->m == 1 )
	{
		ath_yield();
	}
	mtx->m = 1;
}

void ath_mutex_unlock( struct ath_mutex *mtx )
{
	mtx->m = 0;
}

int ath_mutex_is_locked( struct ath_mutex *mtx )
{
	return mtx->m;
}
