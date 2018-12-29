@echo off

Set UNXUTILS_DIR=tools\prebuilt\windows\unxutils

Set ECHO=%UNXUTILS_DIR%\echo.exe
Set CP=%UNXUTILS_DIR%\cp.exe
Set MKDIR=%UNXUTILS_DIR%\mkdir.exe
Set FIND=%UNXUTILS_DIR%\find.exe
Set XARGS=%UNXUTILS_DIR%\xargs.exe
Set RM=%UNXUTILS_DIR%\rm.exe
Set GREP=%UNXUTILS_DIR%\grep.exe
Set SED=%UNXUTILS_DIR%\sed.exe
Set AWK=%UNXUTILS_DIR%\gawk.exe
Set CAT=%UNXUTILS_DIR%\cat.exe
Set SORT=%UNXUTILS_DIR%\sort.exe
Set HEAD=%UNXUTILS_DIR%\head.exe

Set OUTPUT_DIR=output
Set OUTPUT_TMPDIR=.O
Set INFRA_DIR=%OUTPUT_DIR%\eng\infra
Set WRAPPERS_DIR=%OUTPUT_DIR%\eng\wrappers
Set EXAMPLES_DIR=%OUTPUT_DIR%\eng\examples
Set WRAPPER_DOC=tools\doc\wrapper

Set TMP_VARIABLE_DIR=%OUTPUT_DIR%\eng\tmp

Set MODULES= gen_infra ^
gen_dev_sign ^
gen_mqtt ^
gen_sal ^
gen_dynreg

:: Generate Directory
call:gen_eng_dir

:: Prepare Config Macro In make.settings
%SED% -n "/#/!{/=y/p}" make.settings | %SED% -n "s/=y//gp" | %SED% -n "s/FEATURE_//gp" >> %TMP_VARIABLE_DIR%\MACRO_LIST

:: Generate infra_config.h
call:gen_infra_default

for %%I in (%MODULES%) do ^
call:%%I

:: Genrate wrapper.c
call:gen_wrapper_c

GOTO:EOF

:: Functions

:gen_infra_default

%ECHO% "#ifndef _INFRA_CONFIG_H_" > %INFRA_DIR%\infra_config.h
%ECHO% -e "#define _INFRA_CONFIG_H_\n" >> %INFRA_DIR%\infra_config.h
%SED% -n "s/^/#define /p" %TMP_VARIABLE_DIR%\MACRO_LIST >> %INFRA_DIR%\infra_config.h
%ECHO% -e "\n#endif" >> %INFRA_DIR%\infra_config.h

GOTO:EOF

:gen_eng_dir

%RM% -rf %OUTPUT_DIR%/eng
%MKDIR% -p %INFRA_DIR%
%MKDIR% -p %EXAMPLES_DIR%
%MKDIR% -p %WRAPPERS_DIR%
%MKDIR% -p %TMP_VARIABLE_DIR%
%ECHO% "" > %WRAPPERS_DIR%\wrapper.c

GOTO:EOF

:gen_infra

%SED% -n "/INFRA/p" %TMP_VARIABLE_DIR%\MACRO_LIST >> %TMP_VARIABLE_DIR%\MACRO_INFRA
%ECHO% "extract infra module..."
%CAT% %TMP_VARIABLE_DIR%\MACRO_INFRA
%FIND% . -path %OUTPUT_DIR% -prune -type f -o -name "infra_types.h" | %XARGS% -i %CP% -f {} %INFRA_DIR%
%FIND% . -path %OUTPUT_DIR% -prune -type f -o -name "infra_defs.[ch]" | %XARGS% -i %CP% -f {} %INFRA_DIR%
%FIND% . -path %OUTPUT_DIR% -prune -type f -o -name "infra_compat.h" | %XARGS% -i %CP% -f {} %INFRA_DIR%

for /f "delims=" %%I in (%TMP_VARIABLE_DIR%\MACRO_INFRA) do ^
%FIND% . ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "%%I.[ch]" | %XARGS% -i %CP% -f {} %INFRA_DIR%

GOTO:EOF

:gen_dev_sign

Set M_DEV_SIGN=
for /f "delims=" %%I in ('%SED% -n "/DEV_SIGN/p" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_DEV_SIGN=%%I)
if defined M_DEV_SIGN (%ECHO% -e "\nextract dev_sign module...\n"%M_DEV_SIGN%) else (GOTO:EOF)

Set SRC_DEV_SIGN=
for /f "delims=" %%I in ('%FIND% src ^( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} ^) -prune -type f -o -iname %M_DEV_SIGN% -type d') do (Set SRC_DEV_SIGN=%%I)
if NOT defined SRC_DEV_SIGN (GOTO:EOF)

Set DEV_SIGN_DIR=
for /f "delims=" %%I in ('%ECHO% %SRC_DEV_SIGN% ^| %SED% -n "s/.*\\//p" ^| %SED% -n "s/^/%OUTPUT_DIR%\\eng\\/p"') do (Set DEV_SIGN_DIR=%%I)
%MKDIR% -p %DEV_SIGN_DIR%

%FIND% %SRC_DEV_SIGN% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %DEV_SIGN_DIR%
%FIND% %SRC_DEV_SIGN% -maxdepth 1 -name *example*.c | %XARGS% -i %CP% -f {} %EXAMPLES_DIR%

GOTO:EOF

:gen_mqtt

Set M_MQTT_COMM_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "MQTT_COMM_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_MQTT_COMM_ENABLED=%%I)

Set M_MQTT_DEFAULT_IMPL=
for /f "delims=" %%I in ('%GREP% -w "MQTT_DEFAULT_IMPL" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_MQTT_DEFAULT_IMPL=%%I)

Set M_MAL_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "MAL_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_MAL_ENABLED=%%I)

Set M_MAL_ICA_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "MAL_ICA_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_MAL_ICA_ENABLED=%%I)

if defined M_MQTT_COMM_ENABLED (%ECHO% -e "\nextract mqtt module...") else (GOTO:EOF)
%GREP% -E "MQTT|MAL" %TMP_VARIABLE_DIR%\MACRO_LIST

:: extract src/mqtt
Set SRC_MQTT_SIGN=
for /f "delims=" %%I in ('%FIND% src ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "mqtt" -type d') do (Set SRC_MQTT_SIGN=%%I)
if NOT defined SRC_MQTT_SIGN (GOTO:EOF)

%MKDIR% -p %OUTPUT_DIR%/eng/mqtt

%FIND% %SRC_MQTT_SIGN% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %OUTPUT_DIR%/eng/mqtt
if defined M_MQTT_DEFAULT_IMPL (%FIND% %SRC_MQTT_SIGN% -name mqtt_impl -type d | %XARGS% -i %CP% -rf {} %OUTPUT_DIR%/eng/mqtt)
if defined M_MQTT_DEFAULT_IMPL (%FIND% %SRC_MQTT_SIGN% -maxdepth 1 -name *example.c | %XARGS% -i %CP% -f {} %EXAMPLES_DIR%)

:: extract wrappers/mqtt/mal
if NOT defined M_MAL_ENABLED (GOTO:EOF)

Set SRC_MAL_WRAPPER=
for /f "delims=" %%I in ('%FIND% wrappers ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "mal" -type d') do (Set SRC_MAL_WRAPPER=%%I)
if NOT defined SRC_MAL_WRAPPER (GOTO:EOF)

%MKDIR% -p %WRAPPERS_DIR%/mqtt/mal
%FIND% %SRC_MAL_WRAPPER% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %WRAPPERS_DIR%/mqtt/mal

if NOT defined M_MAL_ICA_ENABLED (GOTO:EOF)
%FIND% %SRC_MAL_WRAPPER% -name ica -type d | %XARGS% -i %CP% -rf {} %WRAPPERS_DIR%/mqtt/mal

GOTO:EOF

:gen_sal

Set M_SAL_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "SAL_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_SAL_ENABLED=%%I)

Set M_SAL_HAL_IMPL_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "SAL_HAL_IMPL_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_SAL_HAL_IMPL_ENABLED=%%I)

if defined M_SAL_ENABLED (%ECHO% -e "\nextract sal module...") else (GOTO:EOF)
%GREP% -E "SAL" %TMP_VARIABLE_DIR%\MACRO_LIST

:: extract wrappers/sal and wrappers/at
Set SRC_SAL=
for /f "delims=" %%I in ('%FIND% wrappers ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "sal" -type d') do (Set SRC_SAL=%%I)
if NOT defined SRC_SAL (GOTO:EOF)

%MKDIR% -p %WRAPPERS_DIR%/sal

%FIND% %SRC_SAL% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %WRAPPERS_DIR%\sal\
%FIND% %SRC_SAL% -name src -type d | %XARGS% -i %CP% -rf {} %WRAPPERS_DIR%\sal
%FIND% %SRC_SAL% -name include -type d | %XARGS% -i %CP%  -rf {} %WRAPPERS_DIR%\sal

if NOT defined M_SAL_HAL_IMPL_ENABLED (GOTO:EOF)

%FIND% %SRC_SAL% -name hal-impl -type d | %XARGS% -i %CP% -rf {} %WRAPPERS_DIR%\sal
%FIND% wrappers ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "at" -type d | %XARGS% -i %CP% -rf {} %WRAPPERS_DIR%
%RM% -f %WRAPPERS_DIR%\at\uart.c

GOTO:EOF

:gen_dynreg

Set M_DYNREG=
for /f "delims=" %%I in ('%GREP% -w "DYNAMIC_REGISTER" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_DYNREG=%%I)

if defined M_DYNREG (%ECHO% -e "\nextract dynamic_register module...") else (GOTO:EOF)
%ECHO% %M_DYNREG%

Set SRC_DYNREG=
for /f "delims=" %%I in ('%FIND% src ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname %M_DYNREG% -type d') do (Set SRC_DYNREG=%%I)
if NOT defined SRC_DYNREG (GOTO:EOF)

Set DYNREG_DIR=
for /f "delims=" %%I in ('%ECHO% %SRC_DYNREG% ^| %SED% -n "s/.*\\//p" ^| %SED% -n "s/^/%OUTPUT_DIR%\\eng\\/p"') do (Set DYNREG_DIR=%%I)

%MKDIR% -p %DYNREG_DIR%

%FIND% %SRC_DYNREG% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %DYNREG_DIR%
%FIND% %SRC_DYNREG% -maxdepth 1 -name *example*.c | %XARGS% -i %CP% -f {} %EXAMPLES_DIR%
    
GOTO:EOF

:gen_wrapper_c

%FIND% %OUTPUT_DIR%\eng -name *wrapper.h | %XARGS% -i %GREP% -ro "HAL_.*(" {} | %SED% "s/(//g" | %SORT% -u > %TMP_VARIABLE_DIR%\WRAPPER_FUNCS
%FIND% %OUTPUT_DIR%\eng -name *wrapper.h | %XARGS% -i %GREP% -ro "wrapper_.*(" {} | %SED% "s/(//g" | %SORT% -u >> %TMP_VARIABLE_DIR%\WRAPPER_FUNCS

if defined M_MQTT_COMM_ENABLED (
    if defined M_MQTT_DEFAULT_IMPL (
        %SED% -i "/wrapper_mqtt/d" %TMP_VARIABLE_DIR%\WRAPPER_FUNCS
    )
)

%ECHO% -e "#include \"infra_types.h\"" >> %WRAPPERS_DIR%\wrapper.c
%ECHO% -e "#include \"infra_defs.h\"" >> %WRAPPERS_DIR%\wrapper.c
%FIND% %OUTPUT_DIR%\eng -name *wrapper.h | %SED% -n "s/.*\\/#include \"/;s/$/\"/p" >> %WRAPPERS_DIR%\wrapper.c

if defined M_MQTT_COMM_ENABLED (
    if NOT defined M_MQTT_DEFAULT_IMPL (
        %ECHO% -e "#include \"mqtt_api.h\"" >> %WRAPPERS_DIR%\wrapper.c
    )
)

:: %AWK% "{print NR,$0}" %TMP_VARIABLE_DIR%\WRAPPER_FUNCS

%ECHO% -e "\n\n" >> %WRAPPERS_DIR%\wrapper.c
%SED% -n  "/WRAPPER_NOTE:/{:a;N;/*\//!ba;p}" %WRAPPER_DOC% | %SED% -n "1d;p" >> %WRAPPERS_DIR%\wrapper.c
%ECHO% -e "\n" >> %WRAPPERS_DIR%\wrapper.c

for /f "delims=" %%I in (%TMP_VARIABLE_DIR%\WRAPPER_FUNCS) do (

%FIND% %OUTPUT_DIR%\eng -name *wrapper.h | %XARGS% -i %CAT% {} 2>nul | %SED% -n "/.*%%I(.*/{/.*);/ba;{:c;N;/.*);/!bc};:a;p;q}" >> %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC
%CAT% %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC | %HEAD% -1 | %AWK% -F "wrapper^|HAL" "{ print $1 }" | %SED% s/[[:space:]]//g >> %TMP_VARIABLE_DIR%\WRAPPER_FUNC_RETURN_TYPE
%SED% -n "/%%I:/{:a;N;/*\//!ba;p}" %WRAPPER_DOC% | %SED% -n "1d;p" >> %WRAPPERS_DIR%\wrapper.c

Set DATA_TYPE=
for /f "delims=" %%J in ('%HEAD% -1 %TMP_VARIABLE_DIR%\WRAPPER_FUNC_RETURN_TYPE') do (Set DATA_TYPE=%%J)

if "%DATA_TYPE%" == "void" (
    %SED% -n "/;/{s/;/\n{\n\treturn;\n}\n\n/g};p" %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC >> %WRAPPERS_DIR%\wrapper.c
) else (
    %SED% -n "/;/{s/;/\n{\n\treturn (%DATA_TYPE%)1;\n}\n\n/g};p" %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC >> %WRAPPERS_DIR%\wrapper.c
)
%ECHO% "" > %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC
%ECHO% "" > %TMP_VARIABLE_DIR%\WRAPPER_FUNC_RETURN_TYPE

)

GOTO:EOF

pause