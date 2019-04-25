#!/bin/bash

mv make.settings .config
sed -i 's/FEATURE_//g;/AWSS_SUPPORT_ROUTER/d;s/AWSS_SUPPORT_PHONEASAP/AWSS_SUPPORT_AHA/g' .config
tools/prebuilt/ubuntu/bin/kconfig-conf --olddefconfig tools/Config.in
sed -i '/^[A-Z]/{s/^/FEATURE_/g}' .config
mv .config make.settings
