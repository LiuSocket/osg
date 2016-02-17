.align 4

.globl nv21_to_argb_div2
.globl _nv21_to_argb_div2

yuvrgb_coeffs:
// CONVERSION VALUES computed with analyze.py
.short 25788, 3051, 7668, 30353

/***************************************************************
 * void nv21_to_argb_div2(int w, int h, uint32_t *dst,         *
 *                        const uint8_t *srcY, int linesizeY,  *
 *                        const uint8_t *srcC, int linesizeC); *
 **************************************************************/
nv21_to_argb_div2:
_nv21_to_argb_div2:

    push                {r4-r12, lr}
    vpush               {q4-q7}
    movw                r12, #:lower16:yuvrgb_coeffs
    movt                r12, #:upper16:yuvrgb_coeffs
    vld1.16             {d1}, [r12]                                     /* d1 = coeffs */
    vmov.i8             d0, #128                                        /* for chroma */
    vmov.i8             d15, #255                                       /* default alpha */
    ldr                 r4, [sp, #104]                                  /* r4 = linesizeY */
    ldr                 r5, [sp, #108]                                  /* r5 = srcC */
    ldr                 r6, [sp, #112]                                  /* r6 = linesizeC */
    add                 r12, r3, r4                                     /* r12 = srcY + linesizeY (srcY2) */

.macro align dst, src, n
    add \dst, \src, #\n-1
    bic \dst, #\n-1
.endm

    /* correct the pointers because we are processing 16 pixels at a time, and
       the width may not be a multiple of 16 */
    align               r9, r0, 16
    sub                 r7, r4, r9                                      /* linesizeY - read_width <- luma line skip */
    add                 r4, r7, r4
    sub                 r6, r6, r9                                      /* linesizeC - read_width <- chroma line skip */
    lsr                 r7, r0, #1
    align               r9, r7, 8
    sub                 r8, r9, r7                                      /*  align(width/2, 8) - width/2 */
    lsl                 r8, r8, #2                                      /* (align(width/2, 8) - width/2) * 4 <- dest line backward */

1:
    mov                 r7, r0                                          /* r7 = width */
2:
    pld                 [r3,  #0xc0]
    pld                 [r12, #0xc0]
    pld                 [r5,  #0xc0]

    vld1.8              {q2}, [r3]!                                     /* q2: 1st luma line */
    vld1.8              {q3}, [r12]!                                    /* q3: 2nd luma line */
    vld2.8              {d2, d3}, [r5]!                                 /* q1: interleaved chroma line */

    vsubl.u8            q14, d2, d0                                     /* q15 = U - 128 */
    vsubl.u8            q15, d3, d0                                     /* q14 = V - 128 */
    vaddl.u8            q6, d4, d6
    vaddl.u8            q9, d5, d7
    vuzp.16             d12, d13
    vuzp.16             d18, d19
    vaddl.u16           q6, d12, d13
    vaddl.u16           q9, d18, d19
    vshl.u32            q6, #12                                         /* q6 =  Y4 << 12 (low) */
    vshl.u32            q9, #12                                         /* q9 =  Y4 << 12 (high) */
    vmov                q5,  q6
    vmov                q10, q9
    vmov                q8,  q6
    vmov                q11, q9
    vmlal.s16           q6,  d30, d1[0]                                 /* (Y4 << 12) + V * 22970               (low,  red)   */
    vmlal.s16           q9,  d31, d1[0]                                 /* (Y4 << 12) + V * 22970               (high, red)   */
    vmlsl.s16           q5,  d28, d1[1]                                 /* (Y4 << 12) - U *  5638                             */
    vmlsl.s16           q10, d29, d1[1]                                 /* (Y4 << 12) - U *  5638                             */
    vmlsl.s16           q5,  d30, d1[2]                                 /* (Y4 << 12) - U *  5638 - V * 11700   (low,  green) */
    vmlsl.s16           q10, d31, d1[2]                                 /* (Y4 << 12) - U *  5638 - V * 11700   (high, green) */
    vmlal.s16           q8,  d28, d1[3]                                 /* (Y4 << 12) + U * 29032               (low,  blue)  */
    vmlal.s16           q11, d29, d1[3]                                 /* (Y4 << 12) + U * 29032               (high, blue)  */
    vqrshrun.s32        d2,  q6, #14
    vqrshrun.s32        d3,  q9, #14                                    /* q1 = ({q6,q9 } + (1<<13)) >> 14 (red) */
    vqrshrun.s32        d4,  q5, #14
    vqrshrun.s32        d5, q10, #14                                    /* q2 = ({q5,q10} + (1<<13)) >> 14 (green) */
    vqrshrun.s32        d6,  q8, #14
    vqrshrun.s32        d7, q11, #14                                    /* q3 = ({q8,q11} + (1<<13)) >> 14 (blue) */
    vqmovn.u16          d14, q1                                         /* red   8b */
    vqmovn.u16          d13, q2                                         /* green 8b */
    vqmovn.u16          d12, q3                                         /* blue  8b */
    vst4.u8             {d12, d13, d14, d15}, [r2]!                     /* save R,G,B,A */

    subs                r7, r7, #16
    bgt                 2b

    sub                 r2, r2, r8                                      /* dst   line backward adjustement */
    add                 r3, r3, r4                                      /* srcY  line forward  adjustement */
    add                 r12, r12, r4                                    /* srcY2 line forward  adjustement */
    add                 r5, r5, r6                                      /* srcC  line forward  adjustement */

    subs                r1, r1, #2                                      /* height -= 2 */
    bgt                 1b

    vpop                {q4-q7}
    pop                 {r4-r12, lr}
    mov                 pc, lr
