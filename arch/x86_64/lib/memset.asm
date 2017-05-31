; x86_64 optimized memset function based on XEOS
; Utilizes SSE/SSE2 instructions, but it's OK because AMD64 specifications
; state that SSE/SSE2 is required

global memset
memset:
    mov r9, rdi                     ; Save pointer for returning later
    test rdi, rdi                   ; Test for a NULL pointer
    jz .exit
    test rdx, rdx                   ; Test for a size of zero
    jz .exit
    pxor xmm0, xmm0                 ; Zero out xmm0 register
    test rsi, rsi                   ; Test if we need to pad register (e.g. 0xDE -> 0xDEDEDEDEDEDEDEDE)
    jz .check_alignment
    .pad:
        mov r8, rsi                 ; Save copy of the original value
        shl rsi, 8                  ; Bit shift by 8 bits (1 byte)
        or rsi, r8                  ; Copy in the value saved in r8
        shl rsi, 8
        or rsi, r8
        shl rsi, 8
        or rsi, r8
        movq xmm0, rsi              ; Move RSI into xmm0 for usage later
        shufps xmm0, xmm0, 0x00     ; Shift and duplicate the bottom half into the upper
    .check_alignment:
        and rdi, 0xFFFFFFFFFFFFFFF0 ; Align to 16-bytes for SSE
        mov rax, r9                 ; Duplicate original address of buffer
        sub rax, rdi                ; Find difference between original and aligned
        test rax, rax               ; Check if they actually are the same (aka already aligned)
        jz .align_dispatch
        mov rcx, 16                 ; Find how many we need to manually copy based on number of misaligned
        sub rcx, rax
    .not_aligned:
        mov rax, rsi                ; Move character into RAX so we can access it as AL
        mov rdi, r9                 ; Restore original buffer pointer
    .not_aligned_loop:
        mov [rdi], al               ; Load AL (character) into location pointed to by RDI
        inc rdi                     ; Increment buffer pointer
        dec rcx                     ; Decrease remainder counter
        dec rdx                     ; Decrease total size counter
        test rcx, rcx               ; Check if we are now aligned
        jz .align_dispatch
        jmp .not_aligned_loop
    .align_dispatch:
        test rdx, rdx               ; Test if we are done. It needs to go here, otherwise we will underflow the value
        jz .exit
        cmp rdx, 128                ; Can we do 128 bytes at a time?
        jge .align_128
        cmp rdx, 64                 ; Can we do 64 bytes at a time?
        jge .align_64
        cmp rdx, 32                 ; Can we do 32 bytes at a time?
        jge .align_32
        cmp rdx, 16                 ; Can we do 16 bytes at a time?
        jge .align_16
        mov rax, rsi                ; We are at the end, let's just manually copy the remainder
        jmp .align_end
    .align_128:
        movdqa [rdi], xmm0
        movdqa [rdi + 16], xmm0
        movdqa [rdi + 32], xmm0
        movdqa [rdi + 48], xmm0
        movdqa [rdi + 64], xmm0
        movdqa [rdi + 80], xmm0
        movdqa [rdi + 96], xmm0
        movdqa [rdi + 112], xmm0
        add rdi, 128
        sub rdx, 128
        jmp .align_dispatch
    .align_64:
        movdqa [rdi], xmm0
        movdqa [rdi + 16], xmm0
        movdqa [rdi + 32], xmm0
        movdqa [rdi + 48], xmm0
        add rdi, 64
        sub rdx, 64
        jmp .align_dispatch
    .align_32:
        movdqa [rdi], xmm0
        movdqa [rdi + 16], xmm0
        add rdi, 32
        sub rdx, 32
        jmp .align_dispatch
    .align_16:
        movdqa [rdi], xmm0
        add rdi, 16
        sub rdx, 16
        jmp .align_dispatch
    .align_end:
        mov [rdi], al               ; Pretty much the same as nonaligned code
        inc rdi
        dec rdx
        test rdx, rdx
        jz .exit
        jmp .align_end
    .exit:
        mov rax, r9                 ; Restore the original RDI value
        ret
