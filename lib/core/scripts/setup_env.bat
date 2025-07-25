REM SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
REM
REM SPDX-License-Identifier: Apache-2.0

@echo off

rem ################################################################################

if not "%~1" == "" setlocal

set SENSCORD_INSTALL_PATH=%~dp0

set PATH=%SENSCORD_INSTALL_PATH%\bin;%PATH%

set SENSCORD_FILE_PATH=%SENSCORD_INSTALL_PATH%\config;%SENSCORD_INSTALL_PATH%\bin\component;%SENSCORD_INSTALL_PATH%\bin\allocator;%SENSCORD_INSTALL_PATH%\bin\connection;%SENSCORD_INSTALL_PATH%\bin\converter;%SENSCORD_INSTALL_PATH%\bin\recorder;%SENSCORD_INSTALL_PATH%\bin\extension

if not "%~1" == "" cmd /c %*
