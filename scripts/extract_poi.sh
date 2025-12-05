#!/usr/bin/env bash
set -e

MAP_DIR="/maps"

if [ -z "$1" ]; then
  echo "Usage: $0 <path-to-osm.pbf>"
  echo "Example:"
  echo "  $0 /app/maps/europe/netherlands/netherlands-latest.osm.pbf"
  exit 1
fi

PBF_PATH="$1"

if [ ! -f "$PBF_PATH" ]; then
  echo "File not found: $PBF_PATH"
  exit 1
fi

# === Input path ===
REGION_DIR=$(dirname "$PBF_PATH")
OUTPUT_DIR="$REGION_DIR/poi"
OUTPUT_FILE="$OUTPUT_DIR/poi.geojson"

mkdir -p "$OUTPUT_DIR"

echo "Extract POI from $PBF_PATH"

# === Filter osmium ===
osmium tags-filter "$PBF_PATH" \
  n/amenity \
  n/tourism \
  n/leisure \
  n/natural \
  n/shop \
  -o "$OUTPUT_DIR/poi_filtered.osm.pbf" --overwrite

echo "Converting GeoJSON..."
osmium export "$OUTPUT_DIR/poi_filtered.osm.pbf" -o "$OUTPUT_FILE" --overwrite

# === Remove temporary file ===
rm "$OUTPUT_DIR/poi_filtered.osm.pbf"

echo "File size: $SIZE"
echo "Result: $OUTPUT_FILE"
