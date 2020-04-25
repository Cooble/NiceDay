@ECHO OFF
echo ""
echo ""
:choice
set /P c=Install BUILD in the same folder as SOURCE!? [y/n]!?
if /I "%c%" EQU "y" goto :somewhere
exit

:somewhere
cmake .
pause
exit