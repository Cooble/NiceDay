@ECHO OFF
echo .
echo .
echo This script will download AdditionalResources from github release v1.2a and put them into res folder
echo !Needs to be called from project root folder!
echo .
echo Type Enter to proceed
pause
curl.exe --output AdditionalResourcesV2.zip -L --url https://github.com/Cooble/NiceDay/releases/download/v1.2/AdditionalResourcesV2.zip
cd .\res
tar.exe -xf ..\AdditionalResourcesV2.zip
del /f readme.txt

cd ..
del /f AdditionalResourcesV2.zip

echo Files successfully extracted into res folder
pause