@echo off
REM Set the paths for the executables and .fbs files
set FLATC_PATH=flatc.exe
set HANDLE_GENERATOR_PATH=HandleGenerator.exe
set PACKET_GENERATOR_PATH=PacketGenerator.exe

REM Set the directory containing the .fbs files (current directory)
set FBS_DIR=%~dp0

REM Change to the directory containing the .fbs files
cd %FBS_DIR%

REM Compile the .fbs files
%FLATC_PATH% -c --cpp --gen-mutable enum.fbs struct.fbs protocol.fbs

REM Execute the HandleGenerator.exe and PacketGenerator.exe
%HANDLE_GENERATOR_PATH%
%PACKET_GENERATOR_PATH%

echo Compilation and generation complete.
pause