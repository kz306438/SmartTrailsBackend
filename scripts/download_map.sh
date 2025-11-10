#!/usr/bin/env bash
set -e

# === Settings ===
BASE_URL="https://download.geofabrik.de"
MAP_DIR="/data"

# === Check arguments ===
if [ -z "$1" ]; then
  echo "Usage: $0 <region-or-url>"
  echo "Пример:"
  echo "  $0 europe/netherlands"
  echo "  $0 https://download.geofabrik.de/europe/germany/berlin-latest.osm.pbf"
  exit 1
fi

INPUT="$1"

# === Define reference and file name ===
if [[ "$INPUT" == http* ]]; then
  URL="$INPUT"
  REL_PATH=$(basename "$URL")
  REGION_PATH="${URL#${BASE_URL}/}"
  REGION_DIR=$(dirname "$REGION_PATH")
else
  REGION_DIR="$INPUT"
  FILE_NAME="$(basename "$REGION_DIR")-latest.osm.pbf"
  URL="$BASE_URL/$REGION_DIR-latest.osm.pbf"
fi

DEST_DIR="$MAP_DIR/$REGION_DIR"
DEST_FILE="$DEST_DIR/$(basename "$URL")"

# === Create directories ===
mkdir -p "$DEST_DIR"

# === Download map ===
if [ -f "$DEST_FILE" ]; then
  echo "File already exist: $DEST_FILE"
else
  echo "Downloading map: $URL"
  wget -q --show-progress -O "$DEST_FILE" "$URL" || {
    echo "Error loading map. Check the region name or URL."
    exit 1
  }
  echo "Map successfuly downloaded into: $DEST_FILE"
fi

# === Проверяем размер ===
FILE_SIZE=$(du -h "$DEST_FILE" | cut -f1)
echo "File size: $FILE_SIZE"
