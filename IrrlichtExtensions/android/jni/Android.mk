LOCAL_PATH := $(call my-dir)/../..
include $(CLEAR_VARS)

LOCAL_MODULE := IrrlichtExtension
COMMON_LIB_NAME := lib$(LOCAL_MODULE).a

LOCAL_CFLAGS := -Wall -fexceptions -frtti -fstrict-aliasing

ifndef NDEBUG
LOCAL_CFLAGS += -g -D_DEBUG
else
LOCAL_CFLAGS += -fexpensive-optimizations -O3
endif

# include 디렉토리 추가
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../Irrlicht/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Common

# 소스 파일 추가
LOCAL_SRC_FILES := \
    AggregateGUIElement.cpp \
    AggregatableGUIElementAdapter.cpp \
    AggregateSkinExtension.cpp \
    BeautifulGUIButton.cpp \
    BeautifulGUIImage.cpp \
    BeautifulGUIText.cpp \
    ChooseFromListDialog.cpp \
    CMBox.cpp \
    ColorSelector.cpp \
    ConstantLanguagePhrases.cpp \
    CommonIniEditor.cpp \
    Drawer2D.cpp \
    DragPlaceGUIElement.cpp \
    DraggableGUIElement.cpp \
    NotificationBox.cpp \
    JoyStickElement.cpp \
    EditBoxDialog.cpp \
    FileSystemItemOrganizer.cpp \
    FlexibleFont.cpp \
    font.cpp \
    GUI.cpp \
    IAggregatableGUIElement.cpp \
    IExtendableSkin.cpp \
    InputSystem.cpp \
    ItemSelectElement.cpp \
    KeyInput.cpp \
    ScrollBar.cpp \
    ScrollBarSkinExtension.cpp \
    UnicodeCfgParser.cpp \
    utilities.cpp \
    TouchKey.cpp \
    TouchKeyboard.cpp \
    Transformation2DHelpers.cpp \
    Triangulate.cpp \
    BeautifulCheckBox.cpp

LOCAL_STATIC_LIBRARIES := android_native_app_glue

# 정적 라이브러리 빌드
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/native_app_glue)