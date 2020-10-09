@ECHO OFF
echo .
echo .
echo This script will download AdditionalResources from github release v1.0a and put them into res folder
echo !Needs to be called from project root folder!
echo .
echo Type Enter to proceed
pause
curl.exe --output AdditionalResources.zip -L --url https://github.com/Cooble/NiceDay/releases/download/v1.0a/AdditionalResources.zip
cd .\res
tar.exe -xf ..\AdditionalResources.zip
del /f readme.txt

cd ..
del /f AdditionalResources.zip

echo Files successfully extracted into res folder
pause