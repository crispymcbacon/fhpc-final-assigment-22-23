#!/bin/bash

# Check if the parameter is provided
if [ -z "$1" ]; then
    echo "Please provide the name."
    exit 1
fi

name=$1
index=1

cd out.nosync
mkdir converted

for i in $name*.pgm; do
  output_file="converted/${name}_${index}.jpeg"
  sips -s format jpeg -s formatOptions 80 "$i" --out "$output_file" > /dev/null
  ((index++))
done

ffmpeg -r 30 -start_number 0 -i "converted/${name}_%d.jpeg" -vf "scale=iw*20:ih*20:flags=neighbor" -vcodec mpeg4 "$1.mp4" -y