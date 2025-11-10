#!/usr/bin/env bash
set -e

if [ $# -lt 5 ]; then
    echo "❌ Usage: $0 <input.osm.pbf> <left> <bottom> <right> <top>"
    echo "Пример:"
    echo "  $0 /app/maps/europe/belarus/belarus-latest.osm.pbf 27.45 53.80 27.70 53.95"
    exit 1
fi

INPUT_FILE="$1"
LEFT="$2"
BOTTOM="$3"
RIGHT="$4"
TOP="$5"

if [ ! -f "$INPUT_FILE" ]; then
    echo "❌ Файл не найден: $INPUT_FILE"
    exit 1
fi

# Папка с входным файлом
INPUT_DIR=$(dirname "$INPUT_FILE")

# Создаём папку cropped внутри той же структуры
OUTPUT_DIR="$INPUT_DIR/cropped"
mkdir -p "$OUTPUT_DIR"

# Имя выходного файла
BASENAME=$(basename "$INPUT_FILE" .osm.pbf)
OUTPUT_FILE="$OUTPUT_DIR/${BASENAME}_cropped.osm.pbf"

echo "✂ Вырезаем область [$LEFT, $BOTTOM, $RIGHT, $TOP] из $INPUT_FILE"
osmium extract --bbox "$LEFT,$BOTTOM,$RIGHT,$TOP" -o "$OUTPUT_FILE" "$INPUT_FILE"

echo "✅ Результат сохранён в $OUTPUT_FILE"
