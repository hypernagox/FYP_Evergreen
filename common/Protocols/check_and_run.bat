@echo off
setlocal

REM 솔루션 디렉터리로 이동
cd /d "%~dp0"

REM 파일의 마지막 수정 시간을 기록할 파일
set "TIMESTAMP_FILE=last_compile_time.txt"

REM 감시할 파일들
set "FILES=enum.fbs struct.fbs protocol.fbs"

REM 현재 시간을 저장
set "LAST_MODIFIED=0"
for %%f in (%FILES%) do (
    for /f "tokens=1-5 delims= " %%a in ('dir /tc "%%f"') do (
        set "LAST_MODIFIED=%%a %%b %%c %%d %%e"
    )
)

REM 이전에 저장된 시간을 불러오기
if exist %TIMESTAMP_FILE% (
    set /p "LAST_COMPILED=<%TIMESTAMP_FILE%"
) else (
    set "LAST_COMPILED=0"
)

REM 파일이 변경되었는지 확인
if "%LAST_MODIFIED%" neq "%LAST_COMPILED%" (
    REM 변경된 경우 배치 파일 실행
    call compile_and_generate.bat

    REM 현재 시간을 저장
    echo %LAST_MODIFIED% > %TIMESTAMP_FILE%
)

endlocal