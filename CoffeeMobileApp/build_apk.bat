@echo off
echo Installing EAS CLI...
call npm install -g eas-cli

echo.
echo Please log in to your Expo account (or create one)...
call npx eas-cli login

echo.
echo Starting Cloud Build for Android APK...
call npx eas-cli build -p android --profile preview

echo.
echo Build process started. If successful, you will get a link to download the APK.
pause
