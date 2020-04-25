xcopy /Y /s /d /i "%~dp0\res" "%~dp0\Build"
cmake -S. -B./Build
pause