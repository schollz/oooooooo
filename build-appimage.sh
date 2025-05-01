#!/bin/bash
set -e

# Check and see if ./appimagetool-x86_64.AppImage exist, otherwise download it
if [ ! -f "./appimagetool-x86_64.AppImage" ]; then
    echo "Downloading appimagetool..."    
    wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi


# Your app name (binary name)
APPNAME="oooooooo"

# Paths
BUILD_DIR="build/clients/oooooooo"
APPDIR="AppDir"

# 1. Clean and set up AppDir
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib"

# 2. Copy your app binary
cp "$BUILD_DIR/$APPNAME" "$APPDIR/usr/bin/"

# 3. Copy runtime libraries
echo "Copying dependencies..."
ldd "$BUILD_DIR/$APPNAME" | awk '/=>/ {print $3}' | grep -vE '^$' | grep -vE '^(/lib|/usr/lib)/(x86_64-linux-gnu/)?(libc|libm|libpthread|libdl|ld-linux|libstdc++)' | while read lib; do
    cp -v "$lib" "$APPDIR/usr/lib/"
done

# 4. Create AppRun
cat > "$APPDIR/AppRun" <<EOF
#!/bin/bash
HERE="\$(dirname "\$(readlink -f "\$0")")"
export LD_LIBRARY_PATH="\$HERE/usr/lib:\$LD_LIBRARY_PATH"
exec "\$HERE/usr/bin/$APPNAME" "\$@"
EOF

chmod +x "$APPDIR/AppRun"

# 5. Patch RPATH
echo "Patching RPATH..."
patchelf --set-rpath '$ORIGIN/../lib' "$APPDIR/usr/bin/$APPNAME"

# 6. Create a basic .desktop file
cat > "$APPDIR/$APPNAME.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APPNAME
Exec=$APPNAME
Icon=$APPNAME
Categories=Audio;
EOF

# 7. Create symlink for icon name
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps/"
if [[ -f "$APPNAME.png" ]]; then
    # 256x256 icon
    cp "$APPNAME.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/"
fi

# Make sure the icon exists in the correct location
cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APPNAME.png" "$APPDIR/$APPNAME.png"

# 8. Build AppImage with a specific output name
echo "Building AppImage..."
OUTPUT_NAME="${APPNAME}.AppImage"
./appimagetool-x86_64.AppImage "$APPDIR" "$OUTPUT_NAME"

echo "Done! 🎉 Your AppImage is ready: $OUTPUT_NAME"