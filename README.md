# PSXDOOM RE

Welcome to the complete reverse engineering of PSX Doom by [GEC], this effort took about 2 and a half years to complete, now with the departure of PsyDoom (https://github.com/BodbDearg/PsyDoom) the goal is to collaborate to your Psx Doom BackPort PC and to correct whatever you have missed.

You need to download and install the Psy-Q SDK http://www.psxdev.net/downloads.html

It is necessary to visit this page for installation since it is a very old software, frequently presenting failures in modern Windows
http://www.psxdev.net/help/psyq_install.html

Once you have completed the installation process, go to copy the files in the PSYQ folder and it is necessary for the compilation.


Now everything is complete, copy the PSXDOOM-RE folder to the root "C:" of your hard drive.

You need to copy the PSXDOOM SLUS_000.77 game files, with everything and their folders, the MOVIE.STR file is impossible to copy in Windows, it is the only file that is previously in the ABIN folder.
You run the MAKEFILE.bat to compile and create the main.exe file, it will be automatically copied to the GAME folder.
In the GAME folder, you run the MAKE_CD.bat file, it will create the CD image, and it will also create 2 necessary files that are "PSXCDABS.C and PSXCDABS.H"
these are automatically copied to the previous folder, this is essential since if you modify the code or the files of the game these will change your LBA address and the file size, which requires compiling it again to avoid getting loading errors.


Notes:
The project is created with CodeBlocks, although it does not serve to compile, but to have the code in order and verification.

The mkpsxiso.exe program is originally created by lameguy64, but the present version is modified by me, to create the files "PSXCDABS.C and PSXCDABS.H".
https://github.com/Lameguy64/mkpsxiso by lameguy64


Special thanks to my brothers for the help to the community in DoomWorld and Kaiser since he is the only one to see the progress of my work and helps me in several occasions.

News:
* No clip cheat code from PsyDoom.
* VRAM Viewer added from Master Edition PsxDoom.
* FPS Counter that can be deactivate/activate in the code itself.
