pushd %~dp0
call build.bat "Visual Studio 16 2019" "C:\Program Files\GRAPHISOFT\API Development Kit 25.3002" "AC25"
popd
exit /b %errorlevel%
