#include <stdio.h>
#include "../include/logger.h"

void author(void)
{
	const char team_name[] = "pa1-jsohane-nveerava";
	cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", team_name);
	cse4589_print_and_log("[%s:END]\n", "AUTHOR");
}
