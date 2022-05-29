pushd %~dp0\..

mkdir Build
mkdir Build\%~3
call cmake -B "Build\%~3" -G "%~1" -A "x64" -DAC_API_DEVKIT_DIR="%~2" . || goto :error
call cmake --build "Build\%~3" --config Debug || goto :error
call cmake --build "Build\%~3" --config Release || goto :error
call copy "Build\%~3\Release\DotbimExporter.apx" "Build\%~3\Release\DotbimExporter_%~3.apx" || goto :error
popd

echo Build Succeeded.
exit /b 0

:error
popd
echo Build Failed with Error %errorlevel%.
exit /b 1
