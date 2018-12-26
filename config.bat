@echo off
chcp 65001

set CONFIG_=FEATURE_

.\tools\prebuilt\windows\kconfig-frontends-3.12.0-windows\kconfig-mconf.exe .\tools\Config.in

if exist .config (
	echo .config is exist
	copy /y .config make.settings
	del /F .config
)

pause