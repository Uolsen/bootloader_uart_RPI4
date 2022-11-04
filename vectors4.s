.globl _start
_start:
    b skip

.space 0x800000-0x80004,0

skip:
    mrs x0, mpidr_el1
    and x0, x0, #3
    cmp x0, #0
    beq run
end:
    wfe
    b end
run:
    mov sp,#0x8000000
    bl loader_main

.globl PUT32
PUT32:
    str w1,[x0]
    ret x30

.globl PUT16
PUT16:
    strh w1,[x0]
    ret x30

.globl PUT8
PUT8:
    strb w1,[x0]
    ret x30

.globl GET32
GET32:
    ldr w0,[x0]
    ret x30

.globl GETPC
GETPC:
    mov x0,lr
    ret x30

.globl BRANCHTO
BRANCHTO:
    br x0
    //b #0x80000
//    ret x0

.globl dummy
dummy:
    ret x30
