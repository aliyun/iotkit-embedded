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
Set WRAPPER_DOC=tools\misc\wrapper

Set TMP_VARIABLE_DIR=%OUTPUT_DIR%\eng\tmp

Set MODULES=gen_infra ^
gen_dev_sign ^
gen_mqtt ^
gen_sal ^
gen_dynreg ^
gen_atparser

:: Generate Directory
call:gen_eng_dir

:: Prepare Config Macro In make.settings
%SED% -n "/#/!{/=y/p}" make.settings | %SED% -n "s/=y//gp" | %SED% -n "s/FEATURE_//gp" >> %TMP_VARIABLE_DIR%\MACRO_LIST

:: Generate infra_config.h
call:gen_infra_default

for %%I in (%MODULES%) do (
call:%%I
)

:: Genrate wrapper.c
call:gen_wrapper_c

:: Remove tmp folder
%RM% -rf %TMP_VARIABLE_DIR%

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

%GREP% "SAL_HAL_IMPL" %TMP_VARIABLE_DIR%\MACRO_LIST > %TMP_VARIABLE_DIR%\MACRO_SAL_HAL_IMPL

if defined M_SAL_ENABLED (%ECHO% -e "\nextract sal module...") else (GOTO:EOF)
%GREP% -E "SAL" %TMP_VARIABLE_DIR%\MACRO_LIST

:: extract wrappers/sal and wrappers/at
Set SRC_SAL=
for /f "delims=" %%I in ('%FIND% src ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "sal" -type d') do (Set SRC_SAL=%%I)
if NOT defined SRC_SAL (GOTO:EOF)

%MKDIR% -p %OUTPUT_DIR%/eng/sal

%FIND% %SRC_SAL% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %OUTPUT_DIR%\eng\sal

%SED% -n "s/SAL_HAL_IMPL_//p" %TMP_VARIABLE_DIR%\MACRO_SAL_HAL_IMPL | %XARGS% -i %FIND% %SRC_SAL% -iname {}.c | %XARGS% -i %CP% -f {} %OUTPUT_DIR%/eng/sal

GOTO:EOF

:gen_atparser

Set M_ATPARSER_ENABLED=
for /f "delims=" %%I in ('%GREP% -w "ATPARSER_ENABLED" %TMP_VARIABLE_DIR%\MACRO_LIST') do (Set M_ATPARSER_ENABLED=%%I)

if defined M_ATPARSER_ENABLED (%ECHO% -e "\nextract atparser module...") else (GOTO:EOF)
%GREP% -E "ATPARSER" %TMP_VARIABLE_DIR%\MACRO_LIST

%MKDIR% -p %OUTPUT_DIR%/eng/sal

Set SRC_ATPARSER=
for /f "delims=" %%I in ('%FIND% external_libs ^( -path %OUTPUT_DIR% -o -path %OUTPUT_TMPDIR% ^) -prune -type f -o -iname "at" -type d') do (Set SRC_ATPARSER=%%I)
if NOT defined SRC_ATPARSER (GOTO:EOF)

%FIND% %SRC_ATPARSER% -maxdepth 1 -name *.[ch] | %GREP% -v example | %XARGS% -i %CP% -f {} %OUTPUT_DIR%\eng\sal

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

    if defined M_MAL_ENABLED (
        %SED% -i "/wrapper_mqtt/d" %TMP_VARIABLE_DIR%\WRAPPER_FUNCS
    )
)

if defined M_SAL_ENABLED (
    %SED% -i "/HAL_TCP/d" %TMP_VARIABLE_DIR%\WRAPPER_FUNCS
)

for /f "delims=" %%I in (%TMP_VARIABLE_DIR%\MACRO_SAL_HAL_IMPL) do (
    if NOT "%%I"=="SAL_HAL_IMPL_NONE" (
        %SED% -i "/HAL_SAL/d" %TMP_VARIABLE_DIR%\WRAPPER_FUNCS
    )
)

%CP% -f wrappers\wrappers_defs.h %WRAPPERS_DIR%\

%ECHO% -e "#include \"infra_types.h\"" >> %WRAPPERS_DIR%\wrapper.c
%ECHO% -e "#include \"infra_defs.h\"" >> %WRAPPERS_DIR%\wrapper.c
%ECHO% -e "#include \"wrappers_defs.h\"" >> %WRAPPERS_DIR%\wrapper.c
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

for /f "delims=" %%i in ('%SED% -n "1,1p" %TMP_VARIABLE_DIR%\WRAPPER_FUNC_RETURN_TYPE') do (
    if "%%i" == "void" (
        %SED% -n "/;/{s/;/\n{\n\treturn;\n}\n\n/g};p" %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC >> %WRAPPERS_DIR%\wrapper.c
    ) else (
        %SED% -n "/;/{s/;/\n{\n\treturn (%%i)1;\n}\n\n/g};p" %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC >> %WRAPPERS_DIR%\wrapper.c
    )
)
%RM% -f %TMP_VARIABLE_DIR%\WRAPPER_FUNC_DEC
%RM% -f %TMP_VARIABLE_DIR%\WRAPPER_FUNC_RETURN_TYPE

)

GOTO:EOF

