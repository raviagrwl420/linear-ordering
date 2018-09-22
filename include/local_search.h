#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include<ranking.h>
#include<linear_order.h>
#include<io.h>
#include<memory_management.h>

#include<algorithm>

LinearOrder localSearch (int, int**, LinearOrder);
LinearOrder localSearchExpensive (int, int**, LinearOrder);
LinearOrder localEnumeration (Ranking&, LinearOrder, int);

#endif