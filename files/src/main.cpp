#include"function.h"
//#include <stdio.h>
//#include <iostream>

int main(int argc, char* argv[])
{
	int ret;
	function_init();
	while (1)
	{
		ret = function_step();
		if (ret != 0)
			break;
	}

	function_exit();
}
