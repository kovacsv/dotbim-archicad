pushd %~dp0
call python build.py --acVersion "25" --devKitDir "C:\Program Files\GRAPHISOFT\API Development Kit 25.3002" --projGenerator "Visual Studio 17 2022"
popd
exit /b %errorlevel%
