
   extern kernel_main
   extern kernel_stack_top
   global kstart

[SECTION .text]
kstart:
        mov esp, kernel_stack_top

        call kernel_main

   .halted_loop:
        hlt
        jmp short .halted_loop