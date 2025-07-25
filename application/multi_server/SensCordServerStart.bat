REM SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
REM
REM SPDX-License-Identifier: Apache-2.0

@echo off
setlocal

rem ################################################################################

for /f "delims=" %%a in ('cd') do @set PROJ_TOP=%%a

set PATH=%PROJ_TOP%;%PROJ_TOP%\bin;%PATH%
set SENSCORD_LIB_PATH=%PROJ_TOP%\bin;%PROJ_TOP%\bin\allocator;%PROJ_TOP%\bin\component;%PROJ_TOP%\bin\connection;%PROJ_TOP%\bin\recorder
set SENSCORD_FILE_PATH=%PROJ_TOP%\config\;%SENSCORD_LIB_PATH%
set HDF5_PLUGIN_PATH=%PROJ_TOP%\bin

set SERVER_CONFIG=%PROJ_TOP%\config\senscord_server.xml
if exist %SERVER_CONFIG% (
  "%PROJ_TOP%\bin\SensCordServer.exe" -f "%SERVER_CONFIG%"
) else (
  echo please put configuration file.
  echo   file : %SERVER_CONFIG%
)
