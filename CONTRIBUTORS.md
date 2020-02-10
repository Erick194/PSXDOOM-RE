PSXDOOM RE contributors (sorted alphabetically)
============================================

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
               
* **[James Haley (Quasar)](https://github.com/haleyjd)**
    * Wrong name identification in the PA_PointOnDivlineSide function, which must be called PA_DivlineSide

* **[Samuel Villarreal (svkaiser)](https://github.com/svkaiser)**

    * Console DOOM reverse engineering, specs & tools:
    https://www.doomworld.com/forum/topic/38608-the-console-doom-hacking-project-console-specs
    * Doom64-EX source code (DOOM 64 was based on PSX DOOM, thus can serve as a reference point for it):
    https://github.com/svkaiser/Doom64EX
