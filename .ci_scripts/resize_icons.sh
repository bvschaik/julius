#!/bin/bash -x

source_folder="../res"
mipmap_folder="julius/src/main/res/mipmap"
image_name="julius.png"
original_image="$source_folder/julius_256.png"

convert -resize 72x72 "$original_image" "%mipmap_folder-hdpi/$image_name"
cp "$source_folder/julius_48.png" "%mipmap_folder-mdpi/$image_name"
convert -resize 96x96 "$original_image" "%mipmap_folder-xhdpi/$image_name"
convert -resize 144x144 "$original_image" "%mipmap_folder-xxhdpi/$image_name"
convert -resize 192x192 "$original_image" "%mipmap_folder-xxxhdpi/$image_name"

java_files_path="src/main/java/org/libsdl/app"

cp "../ext/SDL2/SDL2/android-project/app/$java_files_path/*" "../android/julius/$java_files_path/"