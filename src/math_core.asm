; math_core.asm
section .text
global multiply_vector_asm

multiply_vector_asm:
    ; Input:
    ; rdi: Pointer to output Vec4 (result)
    ; rsi: Pointer to Matrix4 (4x4 matrix)
    ; rdx: Pointer to input Vec4 (point to be transformed)

    ; Load the input vector [x, y, z, w] into xmm0
    movaps xmm0, [rdx]

    ; --- Multiply Matrix Row 0 ---
    movaps xmm1, [rsi]      ; Load first row of matrix
    mulps xmm1, xmm0        ; Multiply elements: [m0*x, m1*y, m2*z, m3*w]
    haddps xmm1, xmm1       ; Horizontal add: [a+b, c+d, a+b, c+d]
    haddps xmm1, xmm1       ; Final sum of row 0 in the first slot of xmm1

    ; --- Multiply Matrix Row 1 ---
    movaps xmm2, [rsi + 16] ; Load second row (16 bytes offset)
    mulps xmm2, xmm0
    haddps xmm2, xmm2
    haddps xmm2, xmm2       ; Final sum of row 1

    ; --- Multiply Matrix Row 2 ---
    movaps xmm3, [rsi + 32] ; Load third row
    mulps xmm3, xmm0
    haddps xmm3, xmm3
    haddps xmm3, xmm3

    ; --- Multiply Matrix Row 3 ---
    movaps xmm4, [rsi + 48] ; Load fourth row
    mulps xmm4, xmm0
    haddps xmm4, xmm4
    haddps xmm4, xmm4

    ; Combine results into one register [R0, R1, R2, R3]
    unpcklps xmm1, xmm2     ; Combine row 0 and 1
    unpcklps xmm3, xmm4     ; Combine row 2 and 3
    movlhps xmm1, xmm3      ; Final result in xmm1

    ; Store result back to the memory address in rdi
    movaps [rdi], xmm1
    ret