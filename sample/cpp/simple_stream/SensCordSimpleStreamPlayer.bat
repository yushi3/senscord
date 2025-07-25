REM SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
REM
REM SPDX-License-Identifier: Apache-2.0

rem ################################################################################

FOR /F "usebackq" %%w IN (`cd`) DO SET NOW_DIR=%%w
set SENSCORD_FILE_PATH=%NOW_DIR%\config;%NOW_DIR%\bin\component;%NOW_DIR%\bin\allocator;%NOW_DIR%\bin\recorder
%NOW_DIR%\bin\SensCordSimpleStreamPlayer.exe
pause
