# CopyPad2
Clipboard history implementation in C

## development (nix)

Need to tell Cmake how to find freetype (assuming its also installed via nix-pkg
```
cmake -B build \
  -DFREETYPE_INCLUDE_DIRS="$(nix-build --no-out-link '<nixpkgs>' -A freetype)/include/freetype2" \
  -DFREETYPE_LIBRARY="$(nix-build --no-out-link '<nixpkgs>' -A freetype)/lib/libfreetype.dylib"
```

Then after configuration, everything ends up under `build/`, building the application can be done by running:
```
cmake --build build

``

