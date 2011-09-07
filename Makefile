#################################################################
#################################################################
#
# Cheepo Pacman
# Makefile for Windows XP
#
#################################################################
#################################################################

A=.lib
O=.obj
E=.exe
S=.c
CC=cl -nologo
CFLAGS=/TC /GL /Wall /D__DEBUG__ /O2 /I "C:\Program Files\GnuWin32\Include" /DNDEBUG
LD=cl -nologo
## LDFLAGS=/MDd /MLd
LDFLAGS=/MD 
VERSION=test
VERSION=0.1-291
BASENAME=pacman_
RELEASEBASE=$(BASENAME)$(VERSION)
RELEASEPATH=..\$(RELEASEBASE)
ZIPNAME=$(RELEASEBASE).zip

LIBS= /link \
	opengl32.lib \
	glu32.lib \
	user32.lib \
	gdi32.lib \
	winmm.lib \
	freetype.lib

OBJS= \
	virtmem$O \
	common$O \
	gameboard$O \
	i_system$O \
	win32\win_main$O \
	win32\win_system$O \
	win32\win_gl$O \
	pacman$O \
	player$O \
	game$O \
	event$O \
	menu$O \
	bytesprite$O \
	sector$O \
	image$O \
	thinker$O \
	render$O \
	random$O \
	font$O \
	pathfind$O \
	splash$O \
	pacmanicon.res

all::	pacman

pacman: pacman$E

pacman$E:: $(OBJS)
	$(LD) $(LDFLAGS) -Fe$@ $(OBJS) $(LIBS)

virtmem$O:		virtmem$S; 	$(CC) $(CFLAGS) -c -Fo$@ virtmem$S
common$O:		common$S; 	$(CC) $(CFLAGS) -c -Fo$@ common$S
i_system$O:		i_system$S; 	$(CC) $(CFLAGS) -c -Fo$@ i_system$S
gameboard$O: 	gameboard$S;	$(CC) $(CFLAGS) -c -Fo$@ gameboard$S
pacman$O: pacman$S; $(CC) $(CFLAGS) -c -Fo$@ pacman$S
player$O: player$S; $(CC) $(CFLAGS) -c -Fo$@ player$S
game$O:   game$S; $(CC) $(CFLAGS) -c   -Fo$@ game$S
event$O:  event$S; $(CC) $(CFLAGS) -c  -Fo$@ event$S
menu$O:	  menu$S; $(CC) $(CFLAGS) -c   -Fo$@ menu$S
render$O: render$S; $(CC) $(CFLAGS) -c -Fo$@ render$S
bytesprite$O: bytesprite$S; $(CC) $(CFLAGS) -c -Fo$@ bytesprite$S
sector$O: sector$S; $(CC) $(CFLAGS) -c -Fo$@ sector$S
font$O: font$S; $(CC) $(CFLAGS) -c -Fo$@ font$S
pathfind$O: pathfind$S; $(CC) $(CFLAGS) -c -Fo$@ pathfind$S
splash$O: splash$S; $(CC) $(CFLAGS) -c -Fo$@ splash$S

win32\win_main$O: win32\win_main$S; $(CC) $(CFLAGS) -c -Fo$@ win32\win_main$S
win32\win_gl$O:	  win32\win_gl$S; $(CC) $(CFLAGS) -c -Fo$@   win32\win_gl$S
win32\win_system$O:	win32\win_system$S; $(CC) $(CFLAGS) -c -Fo$@ win32\win_system$S

pacmanicon.res: pacmanicon.rc; rc.exe /fo"pacmanicon.res" ".\pacmanicon.rc"

# filesystem$O:	filesystem$S; 	$(CC) $(CFLAGS) -c -Fo$@ filesystem$S

clean:
	-@if exist *.obj del *.obj
	-@if exist win32\*.obj del win32\*.obj
	-@if exist pacman.exe del pacman.exe
	-@if exist pacmanicon.res del pacmanicon.res
	-@echo del win32\*.obj *.obj pacman.exe pacmanicon.res

vsclean:
	-@if exist pacman_vs2005.ncb del pacman_vs2005.ncb 
 	-@if exist pacman_vs2005.suo del /ah pacman_vs2005.suo
	-@if exist pacman_vs2003.ncb del pacman_vs2003.ncb 
 	-@if exist pacman_vs2003.suo del /ah pacman_vs2003.suo
	-@if exist pacmanicon.aps del pacmanicon.aps
 	-@if exist pacman_vs2005.vcproj.ARMITAGE.greg.user del pacman_vs2005.vcproj.ARMITAGE.greg.user
	-@if exist pacman.pdb del pacman.pdb
	-@if exist Debug rmdir /S /Q Debug
	-@if exist Release rmdir /S /Q Release
	-@if exist pacman.exe del pacman.exe
	-@echo getting rid of visual studio cruft

info:
	-@echo off
	-@SET count=1
	-@FOR /f "tokens=*" %%G IN ('dir /b') DO SET /a count+=1
	-@SET /a count-=1
	-@echo $(count) files
	-@GOTO :eof

mkreldir:
	-@echo off
	-@FOR /f "tokens=1,2* delims= " %%G IN ('date /T') DO @set D=%%H
	-@FOR /f "tokens=1,2,3* delims=/" %%G IN ('echo %D%') DO @set M=%%I%%G%%H
	-@SET R=$(RELEASEPATH)$M
	-@echo got %R% for the release directory name

test: mkreldir
	-@if exist %R% rmdir /s /q %R%\img\..  
	-@mkdir %R%\img
	-@xcopy /C /R /Y img\*tga %R%\img
	-@xcopy /C /R /Y freetype6.dll %R%\img\..

resources:
	-@"rc.exe /fo"Release/pacmanicon.res" ".\pacmanicon.rc""

release: mkreldir
	-@if exist %R% rmdir /s /q %R%\data\..  
	-@if exist %R% del /f /q %R%
	-@if not exist %R%\data mkdir %R%\data
	-@if not exist %R%\data\img mkdir %R%\data\img
	-@if not exist %R%\data\maps mkdir %R%\data\maps
	-@if not exist %R%\data\font mkdir %R%\data\font
	-@if not exist %R%\data\img\splash mkdir %R%\data\img\splash
	-@nmake
	-@xcopy /C /R /Y data\img\*tga %R%\data\img
	-@xcopy /C /R /Y data\img\splash\*tga %R%\data\img\splash
	-@xcopy /C /R /Y pacman.exe %R%\data\..
	-@xcopy /C /R /Y data\maps\pacman1.map %R%\data\maps
	-@xcopy /C /R /Y data\maps\mspacman2.map %R%\data\maps
	-@xcopy /C /R /Y data\maps\argus.map   %R%\data\maps
	-@xcopy /C /R /Y example-gameboard.map %R%\data\..
	-@xcopy /C /R /Y data\maps\openarea.map %R%\data\maps
	-@xcopy /C /R /Y data\font\* %R%\data\font
	-@xcopy /C /R /Y freetype6.dll %R%\data\..
	-@xcopy /C /R /Y zlib1.dll %R%\data\..
	-@xcopy /C /R /Y README.txt %R%\data\..
	-@xcopy /C /R /Y fullscreen.bat %R%\data\..
	-@cd ..
	-@zip -r $(ZIPNAME) $(RELEASEBASE)  

release_old:
	-@if exist ..\pacman_release rmdir /s /q ..\pacman_release
	-@if exist ..\pacman_release del /f /q ..\pacman_release
	-@nmake all
	-@if not exist ..\pacman_release mkdir ..\pacman_release
	-@move pacman.exe ..\pacman_release
	-@mkdir ..\pacman_release\img
	-@xcopy /C /R /Y img\*tga ..\pacman_release\img
	-@xcopy /C /R /Y pacman1.map ..\pacman_release
	-@mkdir ..\pacman_release\font
	-@xcopy /C /R /Y font\* ..\pacman_release\font
	-@copy freetype6.dll ..\pacman_release
	-@nmake clean



##########################################################################
#
# Make the console map-tester map tester separately
#
##########################################################################

MOBJS= \
 	virtmem_m$O \
	common_m$O \
	gameboard_m$O \
	i_system_m$O 

MCFLAGS=/TC /Wall /D__CONSOLE_APP__ /D__MAPTEST__ /D__DEBUG__ 

virtmem_m$O:		virtmem$S; 	$(CC) $(MCFLAGS) -c -Fo$@ virtmem$S
common_m$O:		common$S; 	$(CC) $(MCFLAGS) -c -Fo$@ common$S
i_system_m$O:		i_system$S; 	$(CC) $(MCFLAGS) -c -Fo$@ i_system$S
gameboard_m$O: 	gameboard$S;	$(CC) $(MCFLAGS) -c -Fo$@ gameboard$S

maptest: maptest$E

maptest$E:: $(MOBJS)
	$(LD) $(LDFLAGS) -Fe$@ $(MOBJS) $(LIBS)


