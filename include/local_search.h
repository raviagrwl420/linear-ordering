#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include<ranking.h>
#include<linear_order.h>
#include<io.h>
#include<memory_management.h>
#include<matrix.h>

#include<algorithm>

LinearOrder localSearch (const Ranking&, LinearOrder);
LinearOrder localSearchExpensive ( const Ranking&, LinearOrder);
LinearOrder localEnumeration (const Ranking&, LinearOrder, int);

#endif