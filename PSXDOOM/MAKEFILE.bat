@echo off
set PATH=C:\psyq\bin
set PSYQ_PATH=C:\psyq\bin

::remove_all:
	del obj\*.obj
	del asm\*.s
@echo on

::all:
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/d_main.obj  d_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/info.obj  info.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/sprinfo.obj  sprinfo.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/g_game.obj  g_game.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_base.obj  p_base.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_ceilng.obj  p_ceilng.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_change.obj  p_change.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_doors.obj  p_doors.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_enemy.obj  p_enemy.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_floor.obj  p_floor.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_inter.obj  p_inter.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_lights.obj  p_lights.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_map.obj  p_map.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_maputl.obj  p_maputl.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_mobj.obj  p_mobj.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_move.obj  p_move.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_plats.obj  p_plats.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_pspr.obj  p_pspr.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_setup.obj  p_setup.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_shoot.obj  p_shoot.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_sight.obj  p_sight.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_slide.obj  p_slide.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_spec.obj  p_spec.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_switch.obj  p_switch.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_telept.obj  p_telept.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_tick.obj  p_tick.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/p_user.obj  p_user.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/r_phase1.obj  r_phase1.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/r_phase2.obj  r_phase2.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/r_data.obj  r_data.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/r_main.obj  r_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/vsprintf.obj  vsprintf.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/w_wad.obj  w_wad.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/psx_file.obj  psx_file.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/z_zone.obj  z_zone.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/psxmain.obj  psxmain.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/l_main.obj  l_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/t_main.obj  t_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/m_main.obj  m_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/c_main.obj  c_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/pw_main.obj  pw_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/cf_main.obj  cf_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/m_password.obj  m_password.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/st_main.obj  st_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/am_main.obj  am_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/in_main.obj  in_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/f_main.obj  f_main.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/o_main.obj  o_main.c
	::ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/m_fixed.obj  m_fixed.c
	asmpsx /l m_fixed.s,obj\m_fixed.obj
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/tables.obj  tables.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/psxcd.obj  psxcd.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/s_sound.obj  s_sound.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/psxspu.obj  psxspu.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/psxcmd.obj  psxcmd.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessapip.obj  wessapip.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessapi.obj  wessapi.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessarc.obj  wessarc.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessapit.obj  wessapit.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessseq.obj  wessseq.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/seqload.obj  seqload.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/lcdload.obj  lcdload.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessapim.obj  wessapim.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/seqloadr.obj  seqloadr.c
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj/wessbase.obj  wessbase.c
	asmpsx /l snmain.s,obj\snmain.obj

::child:
	ccpsx -comments-c++ -Wreturn-type -c -O2 -g -o obj\psxmain.obj  psxmain.c
	psylink /m /g /c /q /o$80010000 @main.lnk,main.cpe,main.map
	::cpe2x main.cpe
	cpe2x_gec main.cpe

@echo off
::asm:
	ccpsx -O2 -S -Xo$80010000 -o asm\d_main.s d_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\info.s info.c
	ccpsx -O2 -S -Xo$80010000 -o asm\sprinfo.s sprinfo.c
	ccpsx -O2 -S -Xo$80010000 -o asm\g_game.s g_game.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_base.s p_base.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_ceilng.s p_ceilng.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_change.s p_change.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_doors.s p_doors.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_enemy.s p_enemy.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_floor.s p_floor.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_inter.s p_inter.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_lights.s p_lights.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_map.s p_map.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_maputl.s p_maputl.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_mobj.s p_mobj.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_move.s p_move.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_plats.s p_plats.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_pspr.s p_pspr.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_setup.s p_setup.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_shoot.s p_shoot.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_sight.s p_sight.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_slide.s p_slide.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_spec.s p_spec.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_switch.s p_switch.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_telept.s p_telept.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_tick.s p_tick.c
	ccpsx -O2 -S -Xo$80010000 -o asm\p_user.s p_user.c
	ccpsx -O2 -S -Xo$80010000 -o asm\r_phase1.s r_phase1.c
	ccpsx -O2 -S -Xo$80010000 -o asm\r_phase2.s r_phase2.c
	ccpsx -O2 -S -Xo$80010000 -o asm\r_data.s r_data.c
	ccpsx -O2 -S -Xo$80010000 -o asm\r_main.s r_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\vsprintf.s vsprintf.c
	ccpsx -O2 -S -Xo$80010000 -o asm\w_wad.s  w_wad.c
	ccpsx -O2 -S -Xo$80010000 -o asm\psx_file.s  psx_file.c
	ccpsx -O2 -S -Xo$80010000 -o asm\z_zone.s z_zone.c
	ccpsx -O2 -S -Xo$80010000 -o asm\psxmain.s psxmain.c
	ccpsx -O2 -S -Xo$80010000 -o asm\l_main.s l_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\t_main.s t_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\m_main.s m_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\c_main.s c_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\pw_main.s pw_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\cf_main.s cf_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\m_password.s m_password.c
	ccpsx -O2 -S -Xo$80010000 -o asm\st_main.s st_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\am_main.s am_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\in_main.s in_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\f_main.s f_main.c
	ccpsx -O2 -S -Xo$80010000 -o asm\o_main.s o_main.c
	::ccpsx -O2 -S -Xo$80010000 -o asm\m_fixed.s m_fixed.c
	ccpsx -O2 -S -Xo$80010000 -o asm\tables.s tables.c
	ccpsx -O2 -S -Xo$80010000 -o asm\psxcd.s psxcd.c
	ccpsx -O2 -S -Xo$80010000 -o asm\s_sound.s s_sound.c
	ccpsx -O2 -S -Xo$80010000 -o asm\psxspu.s psxspu.c
	ccpsx -O2 -S -Xo$80010000 -o asm\psxcmd.s psxcmd.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessapip.s wessapip.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessapi.s wessapi.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessarc.s wessarc.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessapit.s wessapit.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessseq.s wessseq.c
	ccpsx -O2 -S -Xo$80010000 -o asm\seqload.s seqload.c
	ccpsx -O2 -S -Xo$80010000 -o asm\lcdload.s lcdload.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessapim.s wessapim.c
	ccpsx -O2 -S -Xo$80010000 -o asm\seqloadr.s seqloadr.c
	ccpsx -O2 -S -Xo$80010000 -o asm\wessbase.s wessbase.c

@echo on

@echo off
	copy main.exe GAME
@echo on
pause
