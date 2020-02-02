    opt at-,w-,c+,m-

    section	asm

	xdef	FixedMul,FixedDiv

FixedMul:
                xor     t0, a0, a1
                bgtz    a0, loc_8003EEB4
                move    v0, a0
                neg     v0, a0

loc_8003EEB4:
                bgtz    a1, loc_8003EEC0
                move    v1, a1
                neg     v1, a1

loc_8003EEC0:
                multu   v0, v1
                mfhi    a0
                mflo    a1
                sll     v0, a0, 16
                srl     v1, a1, 16
                nop
                addu    a0, v0, v1
                bgez    t0, locret_8003EEE8
                move    v0, a0
                neg     v0, a0

locret_8003EEE8:
                jr      ra
                nop

FixedDiv:
                xor     t0, a0, a1
                bgtz    a0, loc_8003EF00
                move    v0, a0
                neg     v0, a0

loc_8003EF00:
                bgtz    a1, loc_8003EF0C
                move    v1, a1
                neg     v1, a1

loc_8003EF0C:
                lui     a2, 1
                sltu    at, v1, v0
                beqz    at, loc_8003EF30
                and     t2, zero, a2

loc_8003EF1C:
                sll     v1, 1
                sll     a2, 1
                sltu    at, v1, v0
                bnez    at, loc_8003EF1C
                and     t2, zero, a2

loc_8003EF30:

                slt     at, v0, v1
                bnez    at, loc_8003EF44
                nop
                sub     v0, v1
                or      t2, a2

loc_8003EF44:
                sll     v0, 1
                srl     a2, 1
                beqz    a2, loc_8003EF5C
                nop
                bnez    v0, loc_8003EF30
                nop

loc_8003EF5C:
                bgez    t0, locret_8003EF68
                move    v0, t2
                neg     v0, t2

locret_8003EF68:
                jr      ra
                nop
	end
