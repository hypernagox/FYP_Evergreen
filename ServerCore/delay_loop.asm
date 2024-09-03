.code
delay_loop PROC
    mov eax, ecx  ;
myloop:
    dec eax       ;
    jnz myloop    ;
    ret
delay_loop ENDP
END