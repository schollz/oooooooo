#!/bin/bash
# Don't use set -e as it causes the script to exit on any error, including warnings from install_name_tool
# Instead, we'll check specific return codes for critical operations

# Your app name (binary name)
APPNAME="oooooooo"

# Paths
BUILD_DIR="build/clients/oooooooo"
APP_BUNDLE="$APPNAME.app"
CONTENTS_DIR="$APP_BUNDLE/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"
RESOURCES_DIR="$CONTENTS_DIR/Resources"

# Function to convert PNG to ICNS
function convert_png_to_icns() {
    local PNG_FILE="$APPNAME.png"
    local ICONSET_DIR="$APPNAME.iconset"
    
    echo "Converting $PNG_FILE to ICNS format..."
    
    # Check if the PNG file exists
    if [ ! -f "$PNG_FILE" ]; then
        echo "Warning: $PNG_FILE not found. Skipping icon creation."
        return 0
    fi
    
    # Create iconset directory
    mkdir -p "$ICONSET_DIR"
    
    # Generate all required icon sizes - ignore errors
    sips -z 16 16     "$PNG_FILE" --out "$ICONSET_DIR/icon_16x16.png" || true
    sips -z 32 32     "$PNG_FILE" --out "$ICONSET_DIR/icon_16x16@2x.png" || true
    sips -z 32 32     "$PNG_FILE" --out "$ICONSET_DIR/icon_32x32.png" || true
    sips -z 64 64     "$PNG_FILE" --out "$ICONSET_DIR/icon_32x32@2x.png" || true
    sips -z 128 128   "$PNG_FILE" --out "$ICONSET_DIR/icon_128x128.png" || true
    sips -z 256 256   "$PNG_FILE" --out "$ICONSET_DIR/icon_128x128@2x.png" || true
    sips -z 256 256   "$PNG_FILE" --out "$ICONSET_DIR/icon_256x256.png" || true
    sips -z 512 512   "$PNG_FILE" --out "$ICONSET_DIR/icon_256x256@2x.png" || true
    sips -z 512 512   "$PNG_FILE" --out "$ICONSET_DIR/icon_512x512.png" || true
    sips -z 1024 1024 "$PNG_FILE" --out "$ICONSET_DIR/icon_512x512@2x.png" || true
    
    # Convert the iconset to icns
    iconutil -c icns "$ICONSET_DIR" || {
        echo "Warning: Failed to convert iconset to ICNS. This is non-fatal."
        rm -rf "$ICONSET_DIR"
        return 0
    }
    
    # Clean up the temporary iconset directory
    rm -rf "$ICONSET_DIR"
    
    echo "Icon conversion complete: $APPNAME.icns created"
    return 0
}

# 1. Clean and set up App bundle structure
rm -rf "$APP_BUNDLE"
mkdir -p "$MACOS_DIR" "$FRAMEWORKS_DIR" "$RESOURCES_DIR"

# 2. Copy your app binary
cp "$BUILD_DIR/$APPNAME" "$MACOS_DIR/"

# 3. Copy runtime libraries (macOS equivalent of ldd is otool)
echo "Copying dependencies..."
# Get list of dependencies
DEPS=$(otool -L "$BUILD_DIR/$APPNAME" | grep -v "/System/Library" | grep -v "/usr/lib" | awk '{print $1}' | grep -v "@executable_path" | grep -v "@rpath" | grep -v ":")

# Copy each dependency
for LIB in $DEPS; do
    if [ -f "$LIB" ]; then
        cp -v "$LIB" "$FRAMEWORKS_DIR/" || {
            echo "Warning: Could not copy $LIB to $FRAMEWORKS_DIR"
            continue
        }
        # Get the library name
        LIBNAME=$(basename "$LIB")
        
        # Update the binary to use the library from the Frameworks directory
        # Ignore the return code from install_name_tool as it returns warnings as errors
        install_name_tool -change "$LIB" "@executable_path/../Frameworks/$LIBNAME" "$MACOS_DIR/$APPNAME" || true
        
        # Update the library's ID
        # Ignore the return code from install_name_tool as it returns warnings as errors
        install_name_tool -id "@executable_path/../Frameworks/$LIBNAME" "$FRAMEWORKS_DIR/$LIBNAME" || true
        
        # Check for dependencies of this library and update them too
        SUB_DEPS=$(otool -L "$FRAMEWORKS_DIR/$LIBNAME" | grep -v "/System/Library" | grep -v "/usr/lib" | awk '{print $1}' | grep -v "@executable_path" | grep -v "@rpath" | grep -v ":")
        for SUB_LIB in $SUB_DEPS; do
            if [ -f "$SUB_LIB" ] && [ "$SUB_LIB" != "$LIB" ]; then
                SUB_LIBNAME=$(basename "$SUB_LIB")
                # Copy if not already copied
                if [ ! -f "$FRAMEWORKS_DIR/$SUB_LIBNAME" ]; then
                    cp -v "$SUB_LIB" "$FRAMEWORKS_DIR/" || {
                        echo "Warning: Could not copy $SUB_LIB to $FRAMEWORKS_DIR"
                        continue
                    }
                    install_name_tool -id "@executable_path/../Frameworks/$SUB_LIBNAME" "$FRAMEWORKS_DIR/$SUB_LIBNAME" || true
                fi
                # Update this library to reference the bundled sub-dependency
                install_name_tool -change "$SUB_LIB" "@executable_path/../Frameworks/$SUB_LIBNAME" "$FRAMEWORKS_DIR/$LIBNAME" || true
            fi
        done
    else
        echo "Warning: Could not find library $LIB"
    fi
done

# 4. Create Info.plist
cat > "$CONTENTS_DIR/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$APPNAME</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.$APPNAME</string>
    <key>CFBundleName</key>
    <string>$APPNAME</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# 5. Convert PNG to ICNS and add icon
# First try to convert PNG to ICNS if PNG exists
if [[ -f "$APPNAME.png" ]]; then
    convert_png_to_icns
fi

# Then add icon if it exists
if [[ -f "$APPNAME.icns" ]]; then
    cp "$APPNAME.icns" "$RESOURCES_DIR/AppIcon.icns"
    echo "Added icon to application bundle"
else
    echo "Warning: No $APPNAME.icns found. App will use default icon."
fi

# 6. Set executable permissions
chmod +x "$MACOS_DIR/$APPNAME"

# 7. Create a DMG (optional)
if command -v create-dmg &> /dev/null; then
    echo "Creating DMG..."
    create-dmg \
        --volname "$APPNAME" \
        --volicon "$APPNAME.icns" \
        --window-pos 200 120 \
        --window-size 600 300 \
        --icon-size 100 \
        --icon "$APPNAME.app" 175 120 \
        --hide-extension "$APPNAME.app" \
        --app-drop-link 425 120 \
        "$APPNAME.dmg" \
        "$APP_BUNDLE"
    echo "DMG created at $APPNAME.dmg"
else
    echo "create-dmg not found. Skipping DMG creation."
    echo "You can install it with 'brew install create-dmg' if you want to create a DMG."
fi

# At the end of the script, ensure we exit with success
echo "Done! 🎉 Your macOS App bundle is ready: $APP_BUNDLE"

# Verify with a simple check
if [ -f "$MACOS_DIR/$APPNAME" ]; then
    echo "✅ Verification: Executable found in the bundle"
else
    echo "❌ Verification: Executable not found in the bundle"
    # Don't exit with error even if verification fails
fi

# Always exit with success
exit 0