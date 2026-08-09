@echo off
setlocal
cd /d "%~dp0"

rem -------------------------------------------------------------
rem  CMakeLists.txt 를 읽어 Visual Studio 솔루션을 생성한다.
rem  언리얼의 "Generate Visual Studio project files" 와 같은 역할.
rem
rem  이럴 때 실행한다:
rem    - 처음 clone 했을 때
rem    - build 폴더를 지웠을 때
rem    - 새 주제 폴더(main.cpp)를 추가했을 때
rem -------------------------------------------------------------

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [오류] vswhere 를 찾지 못했습니다. Visual Studio 설치를 확인하세요.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do set "VSPATH=%%i"
set "CMAKE=%VSPATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist "%CMAKE%" (
    echo [오류] cmake 를 찾지 못했습니다:
    echo   %CMAKE%
    echo Visual Studio 설치 관리자에서 "C++ CMake 도구"를 추가하세요.
    pause
    exit /b 1
)

echo Visual Studio : %VSPATH%
echo.
echo 솔루션 생성 중...
echo.

"%CMAKE%" -B build -S .
if errorlevel 1 (
    echo.
    echo [실패] 위 오류를 확인하세요.
    pause
    exit /b 1
)

echo.
echo ==========================================================
echo  완료: build\engine_lab.slnx
echo.
echo  Visual Studio 에서 열린 뒤 반드시:
echo    1) object_layout 우클릭 -^> 시작 프로젝트로 설정
echo    2) Ctrl+F5 로 실행  (F5 는 콘솔이 바로 닫힘)
echo ==========================================================
echo.

choice /c YN /n /m "지금 솔루션을 열까요? (Y/N) "
if errorlevel 2 goto :end
start "" "build\engine_lab.slnx"

:end
endlocal
