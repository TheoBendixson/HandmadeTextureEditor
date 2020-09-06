echo Building Texture Editor

mkdir ../build
pushd ../build

MAC_PLATFORM_LAYER_PATH="../mac_platform_layer"

echo Compiling Shader Libraries
xcrun -sdk macosx metal -c "${MAC_PLATFORM_LAYER_PATH}/FlatColorShaders.metal" -o ColorShaders.air
xcrun -sdk macosx metallib ColorShaders.air -o ColorShaders.metallib

xcrun -sdk macosx metal -c "${MAC_PLATFORM_LAYER_PATH}/TextureShader.metal" -o TextureShader.air
xcrun -sdk macosx metallib TextureShader.air -o TextureShader.metallib

COMPILER_WARNING_FLAGS="-Werror -Weverything"

DISABLED_ERRORS="-Wno-gnu-anonymous-struct 
                 -Wno-c++11-compat-deprecated-writable-strings                
                 -Wno-cast-qual
                 -Wno-missing-braces                
                 -Wno-pedantic
                 -Wno-unused-variable
                 -Wno-nested-anon-types
                 -Wno-old-style-cast
                 -Wno-unused-value
                 -Wno-unused-macros
                 -Wno-padded
                 -Wno-unused-function
                 -Wno-missing-prototypes
                 -Wno-unused-parameter
                 -Wno-implicit-atomic-properties
                 -Wno-objc-missing-property-synthesis
                 -Wno-nullable-to-nonnull-conversion
                 -Wno-direct-ivar-access
                 -Wno-sign-conversion
                 -Wno-sign-compare
                 -Wno-double-promotion
                 -Wno-tautological-compare
                 -Wno-c++11-long-long
                 -Wno-cast-align"

OSX_LD_FLAGS="-framework AppKit 
              -framework IOKit
              -framework Metal
              -framework MetalKit
              -framework QuartzCore"

COMMON_COMPILER_FLAGS="$COMPILER_WARNING_FLAGS
                       $DISABLED_ERRORS
                       -DCLANG=1
                       -DINTERNAL=1
                       -DLAPTOP=1
                       $OSX_LD_FLAGS"

APP_NAME="TextureEditor"
APP_NAME_WITH_EXTENSION="${APP_NAME}.app"

echo Compiling Texture Editor Mac Platform Layer
clang -g -lstdc++ ${COMMON_COMPILER_FLAGS} -o $APP_NAME "${MAC_PLATFORM_LAYER_PATH}/osx_main.mm"

echo Building Application Bundle
BUNDLE_RESOURCES_PATH="${APP_NAME_WITH_EXTENSION}/Contents/Resources"

rm -rf $APP_NAME_WITH_EXTENSION 
mkdir -p $BUNDLE_RESOURCES_PATH
cp $APP_NAME "${APP_NAME_WITH_EXTENSION}/${APP_NAME}"

cp ColorShaders.metallib ${BUNDLE_RESOURCES_PATH}/ColorShaders.metallib
cp TextureShader.metallib ${BUNDLE_RESOURCES_PATH}/TextureShader.metallib

cp ${MAC_PLATFORM_LAYER_PATH}/Info.plist "${APP_NAME_WITH_EXTENSION}/Contents/Info.plist"
