pushd %~dp0
call build.bat "Visual Studio 17 2022" "C:\Program Files\GRAPHISOFT\API Development Kit 25.3002" "AC25"
popd
exit /b %errorlevel%
