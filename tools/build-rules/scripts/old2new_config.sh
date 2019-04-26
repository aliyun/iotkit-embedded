#!/bin/bash

mv make.settings .config
sed -i 's/FEATURE_//g;/AWSS_SUPPORT_ROUTER/d;s/AWSS_SUPPORT_PHONEASAP/AWSS_SUPPORT_AHA/g' .config
tools/prebuilt/ubuntu/bin/kconfig-conf -s --olddefconfig tools/Config.in
sed -i '/MUTE/d;/^[A-Z]/{s/^/FEATURE_/g}' .config
sed -i '/^#/{s/^# \([A-Z]\)\(.*is not set\)/# FEATURE_\1\2/g}' .config
sed -i '$ a\FEATURE_SUPPORT_TLS=y' .config
mv .config make.settings
