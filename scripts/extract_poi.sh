#!/usr/bin/env bash
set -e

# === Настройки ===

MAP_DIR="/maps"

if [ -z "$1" ]; then
  echo "❌ Usage: $0 <path-to-osm.pbf>"
  echo "Пример:"
  echo "  $0 maps/europe/netherlands/netherlands-latest.osm.pbf"
  exit 1
fi

PBF_PATH="$1"

if [ ! -f "$PBF_PATH" ]; then
  echo "❌ Файл не найден: $PBF_PATH"
  exit 1
fi

# === Пути вывода ===
REGION_DIR=$(dirname "$PBF_PATH")
OUTPUT_DIR="$REGION_DIR/poi"
OUTPUT_FILE="$OUTPUT_DIR/poi.geojson"

mkdir -p "$OUTPUT_DIR"

echo "🔍 Извлекаю POI из $PBF_PATH"

# === Фильтр osmium ===
# Оставляем все объекты, у которых есть один из интересующих тегов
osmium tags-filter "$PBF_PATH" \
  n/amenity \
  n/tourism \
  n/leisure \
  n/natural \
  n/shop \
  -o "$OUTPUT_DIR/poi_filtered.osm.pbf" --overwrite

echo "🧩 Конвертирую в GeoJSON..."
osmium export "$OUTPUT_DIR/poi_filtered.osm.pbf" -o "$OUTPUT_FILE" --overwrite

# === Очистка временного файла ===
rm "$OUTPUT_DIR/poi_filtered.osm.pbf"

# === Информация о результате ===
COUNT=$(grep -c '"type": "Feature"' "$OUTPUT_FILE" || true)
SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)

echo "✅ Извлечено POI: $COUNT объектов"
echo "📦 Размер файла: $SIZE"
echo "📁 Результат: $OUTPUT_FILE"
