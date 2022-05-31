pushd %~dp0
call python build.py --acVersion "24" --devKitDir "C:\Program Files\GRAPHISOFT\API Development Kit 24.3009"
popd
exit /b %errorlevel%
