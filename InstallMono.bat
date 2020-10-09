@ECHO OFF
echo .
echo .
echo This script will install mono on this machine
echo !Needs to be called from project root folder!
echo .
echo Type Enter to proceed
pause
echo "Downloading MONO_INSTALLER"
curl --output mono-6.10.0.104-x64-0.msi -L https://download.mono-project.com/archive/6.10.0/windows-installer/mono-6.10.0.104-x64-0.msi
echo "Installing MONO"
msiexec -i "mono-6.10.0.104-x64-0.msi" -passive -qn -norestart  
del /f mono-6.10.0.104-x64-0.msi

echo Mono Installed