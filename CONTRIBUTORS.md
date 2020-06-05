
= PSXDOOM RE contributors (sorted alphabetically)
=

* **[Darragh Coy (intacowetrust)](https://github.com/BodbDearg)**

    * Various information and code for PSX DOOM based on his own extensive reverse engineering efforts, including:
    
      * Suggestions for changing names in variables:
      
               cheatfullbright -> viewlighting
               solidseg -> solidsubsectors
               newend-> endsubsector
               numsegs-> numdrawsubsectors
               xtoviewangle -> yslope
               vissprites_tmp ->visspritehead
               lastsprite_p -> next
               
      * Identification of differences in the DrawImage and I_DrawSprite code:
         https://github.com/BodbDearg/PsyDoom/commit/9360bd322bc939a29903f21b19f216931b67f196#r37085143
         
      * Bug identification, on line 259 in p_tick.c:
         https://github.com/BodbDearg/PsyDoom/commit/50862aab3a3511dbc33269ee1249429314a71c18#commitcomment-37125911
         
      * Identification of non-existent lines in the original code:
         https://github.com/BodbDearg/PsyDoom/commit/8b7afc9d06f76c9f7fd00fc2e840107dd79a01de#r37163087
         https://github.com/BodbDearg/PsyDoom/commit/775e02de38cd3bf50e3dfa7173529c6ff783d641#r37185771
         
      * Update Identification from Psx Doom Greatest Hits:
         https://github.com/BodbDearg/PsyDoom/commit/775e02de38cd3bf50e3dfa7173529c6ff783d641#r37185747
         
      * Fire sky width repair in title:
         https://github.com/BodbDearg/PsyDoom/commit/9bd75ff52b517bec0737d946b12db5254a1d0e95
         
      * Identification of error in the CD_TRACK array:
         https://github.com/BodbDearg/PsyDoom/commit/7f75f07502bcac6557c7d026f0188f07704899a6#r37223485
         
      * Identification of non-existent lines, in the first version of Psx Doom, but which were subsequently added in the Greatest Hits version:
         https://github.com/BodbDearg/PsyDoom/commit/0457990ceebdf6e0e5cd9279e63a25b97f96a28c#r37246141
         
      * Line identification that is present in the 'Greatest Hits' version
         https://github.com/BodbDearg/PsyDoom/commit/f258d1713611a4e3ea3766f4e02266f3a0772638#r37268303
         
      * Identification of a small correction on line 265 in am_main.cpp
         https://github.com/BodbDearg/PsyDoom/commit/40816754f7978d4c2b3c34bf0cdb31c8e50abd0b#r37315450
         
      * Error identification in state blocks (else / if), in the files (f_main.c and st_main.c)
         https://github.com/BodbDearg/PsyDoom/commit/f4d6fb8b428ff0262cde02f7c77fdd71dc45d0bc#r37348133
         https://github.com/BodbDearg/PsyDoom/commit/201f293f0473e288c77b2f6e3c1a4c8b622c2968#commitcomment-37368964
         
      * Non-existent lines are blocked in Psx Doom
         https://github.com/BodbDearg/PsyDoom/commit/d60da8761208e3cb137e5b5fb85c6dd3a9ff514c#r37805111
         
      * Wess_malloc : fix function signature
         https://github.com/BodbDearg/PsyDoom/commit/a89ae26943fe08dc213d77bcabc50090c74316f0

      * Return fix in wess_seq_loader_init function
         https://github.com/BodbDearg/PsyDoom/commit/81f2bdc65724d2d7bce01e89900845803edeb957#r38278381

      * Error identification on the line (168) in wess_seq_load_sub function
         https://github.com/BodbDearg/PsyDoom/commit/89b38b23f6e0067d7ac123960c0cdc6064a8ef51#r38286066

      * Changes in data types in variable ptk_stat->psp
         https://github.com/BodbDearg/PsyDoom/commit/3eea78ba5a31da31f1007061f95bd6e32d275754#commitcomment-38312476
         https://github.com/BodbDearg/PsyDoom/commit/b952d4be8126ae1fa16f3dfdb87b190c5db6aabb

      * Changes in (Write_Vlq and Len_Vlq) functions
         https://github.com/BodbDearg/PsyDoom/commit/f5e0d69afeb1dee45b699002ee26fe513ae2271d
         
      * Non-existent return statement identification
         https://github.com/BodbDearg/PsyDoom/commit/1a01906c71ea7aadd5a1393cb0cf3365ed68138e#r38841312
         
      * Identification Missing sum in damagecount
         https://github.com/BodbDearg/PsyDoom/commit/69dd2e3ad910eb1dcf5bd159651aafea7e21e3fa
         
      * Automap: fix a PSX DOOM bug where lines flagged with ML_DONTDRAW
         https://github.com/BodbDearg/PsyDoom/commit/5a9b4059ac7a18b724edc380c26fa3fc6e548f5a
         
      * Range checks, added from Psy Doom to avoid alterations in P_RadiusAttack
         https://github.com/BodbDearg/PsyDoom/commit/107f7d3d91824e06f3b9e5106e3498e98a590ebe
      
      * P_FindNextHighestFloor: rewritten to avoid possible undefined behavior, from PsyDoom
         https://github.com/BodbDearg/PsyDoom/commit/91a3cc54e050d1ebba3572fb935a2625717ee21f

      * P_CheckKeyLock and P_SpawnSpecials: added skull key detection code from Greatest Hits version, also fatal BUFFER OVERFLOW is corrected in the D_memset assignment in activeceilings and activeplats
         https://github.com/BodbDearg/PsyDoom/commit/227fbaddf5237088d236865169cb2cae995043a5
         https://github.com/BodbDearg/PsyDoom/commit/f9e7295f63a2757f68bca66eb83661de71b184e5

      * P_SpawnStrobeFlashFast: Fixed `minlight` and` maxlight` assignments, as they were reversed.
         https://github.com/BodbDearg/PsyDoom/commit/8dec4ccbbd73d278b7b1d0d8e1090b0e39ad539d

      * T_CountdownTimer: Correction of regressive count, the code towards which an extra tick will be delayed to carry out the action
         https://github.com/BodbDearg/PsyDoom/commit/890d7dd90474b837d552b45cd38d737f11f7477f

      * P_UseLines: Another blockmap check ranges is added
         https://github.com/BodbDearg/PsyDoom/commit/ea5f99886ae054ac89c88dc961402b681a5b1562

      * A_CheckReload: Eliminate the return status and change a "void" like in Doom PC, since it's return is unnecessary.
         https://github.com/BodbDearg/PsyDoom/commit/7c6d6d851cd6f7f9ac6c3ddd04ce37c8ca53632b

      * A_Punch and A_Saw: (angle_t) identification is added before (P_Random () - P_Random ()), in order to avoid demo desynchronization
         https://github.com/BodbDearg/PsyDoom/commit/7b3d77fdb689f98808faf7a0d3a77f80556dd079
         https://github.com/BodbDearg/PsyDoom/commit/dc56fa6d91ee6d8f54aa11e9053fafd85ddad7f3

      * Suggestion to create a code to increase the map limit to 255
         https://github.com/BodbDearg/PsyDoom/commit/78436a90bfb8e2318141c13bce2e9c8f313af814

      * Floor rendering fix, from PsyDoom
         https://github.com/BodbDearg/PsyDoom/commit/2a0250ca6b43d920706849d0d9018cc4656cb1f1

* **[Fabien Sanglard](https://github.com/fabiensanglard)**

   * Article **[The Polygons of Doom: PSX](http://fabiensanglard.net/doom_psx/index.html)**
   * Fix cryptic magic number "mode" for Plane rendition
   
* **[James Haley (Quasar)](https://github.com/haleyjd)**
    * Wrong name identification in the PA_PointOnDivlineSide function, which must be called PA_DivlineSide

* **[Samuel Villarreal (svkaiser)](https://github.com/svkaiser)**

    * Console DOOM reverse engineering, specs & tools:
    https://www.doomworld.com/forum/topic/38608-the-console-doom-hacking-project-console-specs
    * Doom64-EX source code (DOOM 64 was based on PSX DOOM, thus can serve as a reference point for it):
    https://github.com/svkaiser/Doom64EX
