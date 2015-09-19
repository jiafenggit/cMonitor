#include <stdio.h>
#include "c_collection.h"

int main(void)
{
	char *tmp = NULL;

	tmp  = collect_sys_info();
	free(tmp);
	return 0;
	return 0;
}

