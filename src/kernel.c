/* kernel.c */
#include "terminal.h"


void kernel_main()
{
    clear_screen();
    gotoxy(5, 1);
    kprints("Starting Kernel...", CYAN, BLACK, 0);


}