LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

#for file in *.cpp; do echo "                   ../../Classes/"$file "\"; done
LOCAL_SRC_FILES := hellocpp/main.cpp \
                   ../../Classes/AI.cpp \
                   ../../Classes/App.cpp \
                   ../../Classes/Block.cpp \
                   ../../Classes/Bullet.cpp \
                   ../../Classes/Dialog.cpp \
                   ../../Classes/GAnimation.cpp \
                   ../../Classes/GObject.cpp \
                   ../../Classes/GSpace.cpp \
                   ../../Classes/Garden.cpp \
                   ../../Classes/Glyph.cpp \
                   ../../Classes/Graphics.cpp \
                   ../../Classes/HUD.cpp \
                   ../../Classes/LuaAPI.cpp \
                   ../../Classes/LuaCWrappers.cpp \
                   ../../Classes/LuaConvert.cpp \
                   ../../Classes/LuaShell.cpp \
                   ../../Classes/LuaWrap.cpp \
                   ../../Classes/ObjectMapping.cpp \
                   ../../Classes/Patchouli.cpp \
                   ../../Classes/PlayScene.cpp \
                   ../../Classes/Player.cpp \
                   ../../Classes/SceneMapping.cpp \
                   ../../Classes/Spell.cpp \
                   ../../Classes/SpellMapping.cpp \
                   ../../Classes/controls.cpp \
                   ../../Classes/menu.cpp \
                   ../../Classes/util.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes \
                    $(LOCAL_PATH)/../../Classes/scenes \
                    $(LOCAL_PATH)/../../cocos2d/chipmunkpp \
                    $(LOCAL_PATH)/../../cocos2d/chipmunk \
                    $(LOCAL_PATH)/../../cocos2d/lua \
                    $(LOCAL_PATH)/../../cocos2d/LuaBridge \
                    /Users/toni/dev/boost_1_59_0

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cocos2dx_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
