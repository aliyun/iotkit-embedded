# Basic Settings
#
SHELL           = /bin/bash
TOP_DIR         ?= $(CURDIR)

# Settings of input directory
#
RULE_DIR        := $(TOP_DIR)/build-rules
CONFIG_DIR      := $(TOP_DIR)/src/configs
SCRIPT_DIR      := $(TOP_DIR)/src/scripts
PACKAGE_DIR     := $(TOP_DIR)/src/packages
IMPORT_DIR      := $(TOP_DIR)/src/import

# Settings of output directory
#
CONFIG_TPL      := $(TOP_DIR)/.config
OUTPUT_DIR      ?= $(TOP_DIR)/.O
DIST_DIR        := $(TOP_DIR)/output
LCOV_DIR        := Coverage
FINAL_DIR       := $(DIST_DIR)/release

# Setting of directory for flat modules
#
bindir          = /usr/bin
libdir          = /usr/lib
incdir          = /usr/include

# Setting of build process control
#
COMPILE_LOG     := compile.log
WARNING_LOG     := warnings.log
HD_MAKEFILE     := makefile
TOP_MAKEFILE    := makefile
MAKE_SEGMENT    := iot.mk

STAMP_PRJ_CFG   := $(OUTPUT_DIR)/.just.configured
STAMP_BLD_ENV   := $(OUTPUT_DIR)/.sub-build.env
STAMP_BLD_VAR   := $(OUTPUT_DIR)/.sub-build.vars
STAMP_UNPACK    := .unpack.done
STAMP_CONFIG    := .config.done
STAMP_BUILD     := .build.done
STAMP_INSTALL   := .install.done
STAMP_POSTINS   := .post.install.done
STAMP_SHIELD    := .shield.done
STAMP_POST_RULE := $(OUTPUT_DIR)/.post-rules.mk

ifdef DEBUG
AUTO_HEADER_Q   :=
ORIGIN_Q        :=
SHELL_DBG       := set -x;
Q               :=
TOP_Q           :=
else
Q               := @
TOP_Q           := @
endif

ifdef DEBUG_SHELL
SHELL_DBG       := set -x;
endif

# Setting of directory for origin modules
#
export INSTALL_DIR     = $(OUTPUT_DIR)/usr
export INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
export INSTALL_LIB_DIR = $(INSTALL_DIR)/lib
