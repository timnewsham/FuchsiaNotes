LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
MODULE_TYPE := userapp
MODULE_SRCS += $(LOCAL_DIR)/test6.c
MODULE_NAME := test6
MODULE_LIBS := system/ulib/magenta system/ulib/mxio system/ulib/c
include make/module.mk
