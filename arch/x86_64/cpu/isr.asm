%macro PUSHA 0
    push rax      ;save current rax
    push rbx      ;save current rbx
    push rcx      ;save current rcx
    push rdx      ;save current rdx
    push rbp      ;save current rbp
    push rdi       ;save current rdi
    push rsi       ;save current rsi
    push r8        ;save current r8
    push r9        ;save current r9
    push r10      ;save current r10
    push r11      ;save current r11
    push r12      ;save current r12
    push r13      ;save current r13
    push r14      ;save current r14
    push r15      ;save current r15
%endmacro

%macro POPA 0
    pop r15      ;restore current r15
    pop r14      ;restore current r14
    pop r13      ;restore current r13
    pop r12      ;restore current r12
    pop r11      ;restore current r11
    pop r10      ;restore current r10
    pop r9        ;restore current r9
    pop r8        ;restore current r8
    pop rsi       ;restore current rsi
    pop rdi       ;restore current rdi
    pop rbp      ;restore current rbp
    pop rdx      ;restore current rdx
    pop rcx      ;restore current rcx
    pop rbx      ;restore current rbx
    pop rax      ;restore current rax
%endmacro

%macro ISR_NOERROR_CODE 1
  [GLOBAL isr%1]
  isr%1:
    push qword 0
    push qword %1
    jmp isr_stub
%endmacro

%macro ISR_ERROR_CODE 1
  [GLOBAL isr%1]
  isr%1:
    push qword %1
    jmp isr_stub
%endmacro

%macro IRQ 2
  [GLOBAL irq%1]
  irq%1:
    push qword 0
    push qword %2
    jmp irq_stub
%endmacro

ISR_NOERROR_CODE 0
ISR_NOERROR_CODE 1
ISR_NOERROR_CODE 2
ISR_NOERROR_CODE 3
ISR_NOERROR_CODE 4
ISR_NOERROR_CODE 5
ISR_NOERROR_CODE 6
ISR_NOERROR_CODE 7
ISR_ERROR_CODE 8
ISR_NOERROR_CODE 9
ISR_ERROR_CODE 10
ISR_ERROR_CODE 11
ISR_ERROR_CODE 12
ISR_ERROR_CODE 13
ISR_ERROR_CODE 14
ISR_NOERROR_CODE 15 ; Reserved
ISR_NOERROR_CODE 16
ISR_ERROR_CODE 17
ISR_NOERROR_CODE 18
ISR_NOERROR_CODE 19
ISR_NOERROR_CODE 20
ISR_NOERROR_CODE 21 ; Reserved
ISR_NOERROR_CODE 22 ; Reserved
ISR_NOERROR_CODE 23 ; Reserved
ISR_NOERROR_CODE 24 ; Reserved
ISR_NOERROR_CODE 25 ; Reserved
ISR_NOERROR_CODE 26 ; Reserved
ISR_NOERROR_CODE 27 ; Reserved
ISR_NOERROR_CODE 28 ; Reserved
ISR_NOERROR_CODE 29 ; Reserved
ISR_ERROR_CODE 30
ISR_NOERROR_CODE 31
; IRQs
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

extern x86_64_exception_handler
isr_stub:
    cld

    PUSHA

    mov rdi, rsp                        ; Push the registers
    call x86_64_exception_handler

    POPA
    add rsp, 16
    iretq

extern x86_64_interrupt_handler
irq_stub:
    cld

    PUSHA

    mov rdi, rsp                        ; Push the registers
    call x86_64_interrupt_handler

    POPA
    add rsp, 16
    iretq

global idt_load
idt_load:
    lidt [rdi]
    ret
