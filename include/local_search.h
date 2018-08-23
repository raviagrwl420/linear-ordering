#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include<ranking.h>
#include<linear_order.h>
#include<io.h>

LinearOrder localSearch (int, int**, LinearOrder);
LinearOrder localSearchExpensive (int, int**, LinearOrder);

#endif