xcopy /Y /s /d /i "%~dp0\res" "%~dp0\Build\res"
cmake -S. -B./Build
pause