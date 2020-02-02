    opt at-,w-,c+,m-

    section	asm
	include	global.inc

	xdef	TextureCache,V_ClearBlock2,Valloc_Init2

	xref	I_Error,error_t,CDVolume,V_PagFlags,PageCount,vram_cache,xcount,ycount,xycount,TextureCacheIdx
	xref	W_CacheLumpNum,lumpencode,tempbuffer,decode,LoadImage,GetTPage,V_ClearBlock,PagesXY

TextureCache:
                lw      v0, TextureCacheIdx-GP_ADDR(gp)
                addiu   sp, -0x58
                sw      s3, 0x44(sp)
                move    s3, a0
                sw      ra, 0x50(sp)
                sw      s5, 0x4C(sp)
                sw      s4, 0x48(sp)
                sw      s2, 0x40(sp)
                sw      s1, 0x3C(sp)
                sw      s0, 0x38(sp)
                lhu     v1, 0xA(s3)
                nop
                bnez    v1, loc_80033758
                sw      v0, 0x1C(s3)
                lw      s5, PageCount-GP_ADDR(gp)

loc_80033428:

                lh      v0, 0xC(s3)
                lw      v1, PageCount-GP_ADDR(gp)
                nop
                addu    v0, v0, v1
                slti    v0, v0, 0x11
                bnez    v0, loc_8003345C
                nop
                lw      v0, ycount-GP_ADDR(gp)
                lw      v1, xycount-GP_ADDR(gp)
                sw      zero, PageCount-GP_ADDR(gp)
                sw      zero, xycount-GP_ADDR(gp)
                addu    v0, v0, v1
                sw      v0, ycount-GP_ADDR(gp)

loc_8003345C:
                lh      v0, 0xE(s3)
                lw      v1, ycount-GP_ADDR(gp)
                nop
                addu    v0, v0, v1
                slti    v0, v0, 0x11
                bnez    v0, loc_800334F0
                move    s2, zero
                li      a2, 0x2E8BA2E9
                lw      a1, V_PagFlags-GP_ADDR(gp)

loc_80033484:
                lw      a0, PageCount-GP_ADDR(gp)
                nop
                addiu   a0, a0, 1
                mult    a0, a2
                sra     v0, a0, 31
                mfhi    v1
                sra     v1, v1, 1
                subu    v1, v0
                sll     v0, v1, 1
                addu    v0, v1
                sll     v0, v0, 2
                subu    v0, v1
                subu    a0, v0
                srav    v0, a1, a0
                andi    v0, v0, 1
                sw      a0, PageCount-GP_ADDR(gp)
                bnez    v0, loc_80033484
                nop
                bne     a0, s5, loc_800334E4
                move    s2, zero
                ;la      a0, PageCount;msg Texture Cache Overflow\n
                jal     I_Error
                nop

loc_800334E4:
                sw      zero, PageCount-GP_ADDR(gp)
                sw      zero, ycount-GP_ADDR(gp)
                sw      zero, xycount-GP_ADDR(gp)

loc_800334F0:
                lw      v1, PageCount-GP_ADDR(gp)
                lw      v0, vram_cache-GP_ADDR(gp)
                lw      a0, ycount-GP_ADDR(gp)
                sll     v1, v1, 10
                addu    v1, v1, v0
                sll     a0, a0, 6
                lw      v0, PageCount-GP_ADDR(gp)
                addu    v1, v1, a0
                sll     v0, v0, 2
                addu    s4, v1, v0
                lh      v0, 0xE(s3)
                nop
                blez    v0, loc_800335CC
                move    s1, s4

loc_80033528:
                lh      v0, 0xC(s3)
                nop
                blez    v0, loc_800335A8
                move    s0, zero

loc_80033538:
                lw      a1, 0(s1)
                nop
                beqz    a1, loc_80033594
                addiu   s1, s1, 4
                lw      v1, 0x1C(a1)
                lw      v0, TextureCacheIdx-GP_ADDR(gp)
                nop
                bne     v1, v0, loc_8003358C
                nop
                lh      v0, 0xC(a1)
                lw      a0, PageCount-GP_ADDR(gp)
                lh      a1, 0xE(a1)
                lw      v1, xycount-GP_ADDR(gp)
                addu    v0, v0, a0
                slt     v1, v1, a1
                sw      v0, PageCount-GP_ADDR(gp)
                beqz    v1, loc_80033428
                nop
                sw      a1, xycount-GP_ADDR(gp)
                j       loc_80033428
                nop

loc_8003358C:
                jal     V_ClearBlock
                move    a0, a1

loc_80033594:
                lh      v0, 0xC(s3)
                addiu   s0, s0, 1
                slt     v0, s0, v0
                bnez    v0, loc_80033538
                nop

loc_800335A8:
                li      v0, 0x10
                subu    v0, v0, s0
                sll     v0, v0, 2
                addu    s1, s1, v0
                lh      v0, 0xE(s3)
                addiu   s2, s2, 1
                slt     v0, s2, v0
                bnez    v0, loc_80033528
                nop

loc_800335CC:
                move    s1, s4
                lh      v0, 0xE(s3)
                nop
                blez    v0, loc_8003362C
                move    s2, zero
                li      v1, 0x10

loc_800335E4:
                lh      v0, 0xC(s3)
                nop
                blez    v0, loc_8003360C
                move    s0, zero

loc_800335F4:
                sw      s3, 0(s1)
                lh      v0, 0xC(s3)
                addiu   s0, s0, 1
                slt     v0, s0, v0
                bnez    v0, loc_800335F4
                addiu   s1, s1, 4

loc_8003360C:
                subu    v0, v1, s0
                sll     v0, v0, 2
                addu    s1, s1, v0
                lh      v0, 0xE(s3)
                addiu   s2, s2, 1
                slt     v0, s2, v0
                bnez    v0, loc_800335E4
                nop

loc_8003362C:
                li      a1, 0x20
                lh      a0, 0x10(s3)
                jal     W_CacheLumpNum
                move    a2, zero
                lh      v1, 0x10(s3)
                lw      a0, lumpencode
                nop
                addu    a0, a0, v1
                lbu     v1, 0(a0)
                nop
                bnez    v1, loc_80033674
                move    s0, v0
                move    a0, s0
                la      s0, tempbuffer
                jal     decode
                move    a1, s0

loc_80033674:
                lw      a0, PageCount-GP_ADDR(gp)
                lw      v0, PageCount-GP_ADDR(gp)
                sw      s4, 0x14(s3)
                sll     a0, a0, 3
                ;li      at, 0x80073AAC
                la      at, PagesXY
                addu    at, at, a0
                lhu     v1, 0(at)
                sll     v0, 3
                addu    v1, v0
                lw      v0, ycount-GP_ADDR(gp)
                sh      v1, 0x10(sp)
                ;li      at, 0x80073AB0
                la      at, PagesXY+4
                addu    at, a0
                lhu     v1, 0(at)
                sll     v0, 4
                addu    v1, v0
                sh      v1, 0x12(sp)
                lhu     v0, 4(s3)
                addiu   a1, s0, 8
                sll     v0, 16
                sra     v0, 17
                sh      v0, 0x14(sp)
                lhu     v0, 6(s3)
                addiu   a0, sp, 0x10
                jal     LoadImage
                sh      v0, 0x16(sp)
                lw      v0, PageCount-GP_ADDR(gp)
                nop
                sll     v0, 4
                sb      v0, 8(s3)
                lw      v0, ycount-GP_ADDR(gp)
                li      a0, 1
                sll     v0, 4
                sb      v0, 9(s3)
                lw      v0, PageCount-GP_ADDR(gp)
                move    a1, zero
                addiu   a2, v0, 4
                sll     v0, 3
                ;li      at, 0x80073AB0
                la      at, PagesXY+4
                addu    at, v0
                lw      a3, 0(at)
                jal     GetTPage
                sll     a2, 7
                lh      v1, 0xC(s3)
                lw      a0, PageCount-GP_ADDR(gp)
                lh      a1, 0xE(s3)
                sh      v0, 0xA(s3)
                lw      v0, xycount-GP_ADDR(gp)
                addu    v1, a0
                slt     v0, v0, a1
                sw      v1, PageCount-GP_ADDR(gp)
                beqz    v0, loc_80033758
                nop
                sw      a1, xycount-GP_ADDR(gp)

loc_80033758:

                lw      ra, 0x50(sp)
                lw      s5, 0x4C(sp)
                lw      s4, 0x48(sp)
                lw      s3, 0x44(sp)
                lw      s2, 0x40(sp)
                lw      s1, 0x3C(sp)
                lw      s0, 0x38(sp)
                addiu   sp, 0x58
                jr      ra
                nop


V_ClearBlock2:
    addiu   sp, -0x10
    lw      a1, 0x14(a0)
    lh      v0, 0xE(a0)
    move    a2, zero
    blez    v0, loc_800337E4
    sh      zero, 0xA(a0)
    li      a3, 0x10

loc_8003379C:
    lh      v0, 0xC(a0)
    nop
    blez    v0, loc_800337C4
    move    v1, zero

loc_800337AC:
    sw      zero, 0(a1)
    lh      v0, 0xC(a0)
    addiu   v1, 1
    slt     v0, v1, v0
    bnez    v0, loc_800337AC
    addiu   a1, 4

loc_800337C4:
    subu    v0, a3, v1
    sll     v0, 2
    addu    a1, v0
    lh      v0, 0xE(a0)
    addiu   a2, 1
    slt     v0, a2, v0
    bnez    v0, loc_8003379C
    nop

loc_800337E4:
    addiu   sp, 0x10
    jr      ra
    nop


Valloc_Init2:
                addiu   sp, -0x10
    ;la      a0, error_t
    ;lw      v0, CDVolume
    ;move    a1, v0
    ;la      a2, CDVolume-GP_ADDR(gp)
    ;lw      v0, CDVolume-GP_ADDR(gp)
    ;move    a3, v0
    ;jal     I_Error
                move    a3, zero
                li      t2, 0x10

loc_800337FC:
                lw      v1, V_PagFlags-GP_ADDR(gp)
                j       loc_8003380C
                srav    v0, v1, a3

loc_80033808:
                srav    v0, v1, a3

loc_8003380C:
                andi    v0, 1
                bnez    v0, loc_80033808
                addiu   a3, 1
                addiu   a3, -1
                move    t1, zero
                lw      v1, vram_cache-GP_ADDR(gp)
                sll     v0, a3, 10
                addu    t0, v0, v1

loc_8003382C:
                lw      v0, 0(t0)
                nop
                beqz    v0, loc_8003389C
                nop
                move    a1, v0
                lw      a0, 0x14(v0)
                lh      v0, 0xE(a1)
                move    a2, zero
                blez    v0, loc_8003389C
                sh      zero, 0xA(a1)

loc_80033854:
                lh      v0, 0xC(a1)
                nop
                blez    v0, loc_8003387C
                move    v1, zero

loc_80033864:
                sw      zero, 0(a0)
                lh      v0, 0xC(a1)
                addiu   v1, 1
                slt     v0, v1, v0
                bnez    v0, loc_80033864
                addiu   a0, 4

loc_8003387C:
                subu    v0, t2, v1
                sll     v0, 2
                addu    a0, v0
                lh      v0, 0xE(a1)
                addiu   a2, 1
                slt     v0, a2, v0
                bnez    v0, loc_80033854
                nop

loc_8003389C:

                addiu   t1, 1
                slti    v0, t1, 0x100
                bnez    v0, loc_8003382C
                addiu   t0, 4
                addiu   a3, 1
                slti    v0, a3, 0xB
                bnez    v0, loc_800337FC
                nop
                lw      a1, V_PagFlags-GP_ADDR(gp)
                sw      zero, PageCount-GP_ADDR(gp)
                andi    v0, a1, 1
                beqz    v0, loc_80033920
                nop
                li      a2, 0x2E8BA2E9

loc_800338D8:
                lw      a0, PageCount-GP_ADDR(gp)
                nop
                addiu   a0, 1
                mult    a0, a2
                sra     v0, a0, 31
                mfhi    v1
                sra     v1, 1
                subu    v1, v0
                sll     v0, v1, 1
                addu    v0, v1
                sll     v0, 2
                subu    v0, v1
                subu    a0, v0
                sw      a0, PageCount-GP_ADDR(gp)
                srav    a0, a1, a0
                andi    a0, 1
                bnez    a0, loc_800338D8
                nop

loc_80033920:
                sw      zero, xcount-GP_ADDR(gp)
                sw      zero, ycount-GP_ADDR(gp)
                sw      zero, xycount-GP_ADDR(gp)
                addiu   sp, 0x10
                jr      ra
                nop
