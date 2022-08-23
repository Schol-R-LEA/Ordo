
   extern kernel_main
   global kstart

[SECTION .text]
kstart:
        call kernel_main

   .halted_loop:
        hlt
        jmp short .halted_loop