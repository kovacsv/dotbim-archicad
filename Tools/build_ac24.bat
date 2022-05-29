pushd %~dp0
call build.bat "Visual Studio 15 2017" "C:\Program Files\GRAPHISOFT\API Development Kit 24.3009" "AC24"
popd
exit /b %errorlevel%
