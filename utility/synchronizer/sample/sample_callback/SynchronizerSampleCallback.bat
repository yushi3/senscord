REM SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
REM
REM SPDX-License-Identifier: Apache-2.0

rem ################################################################################

for /f "delims=" %%a in ('cd') do @set PROJ_TOP=%%a
set PATH=%PATH%;%PROJ_TOP%\bin
set SENSCORD_LIB_PATH=%PROJ_TOP%\bin;%PROJ_TOP%\bin\allocator;%PROJ_TOP%\bin\component;%PROJ_TOP%\bin\recorder;%PROJ_TOP%\bin\connection
set SENSCORD_FILE_PATH=;%PROJ_TOP%\config;%SENSCORD_LIB_PATH%
%PROJ_TOP%\bin\utility\SynchronizerSampleCallback.exe
pause
