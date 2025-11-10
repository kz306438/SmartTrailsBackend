#!/usr/bin/env bash
set -e

TRIGGER_FILE="/app/maps/update.trigger"
CURRENT_PID=0

echo "👀 OSRM Watcher (MLD) запущен. Жду триггер: $TRIGGER_FILE"

while true; do
    if [ -f "$TRIGGER_FILE" ]; then
        # Читаем полный путь к osm.pbf
        PBF_PATH=$(cat "$TRIGGER_FILE" | tr -d '[:space:]')

        if [ -z "$PBF_PATH" ]; then
            echo "❌ Триггер пустой, нужно указать полный путь к .osm.pbf"
            rm -f "$TRIGGER_FILE"
            sleep 5
            continue
        fi

        if [ ! -f "$PBF_PATH" ]; then
            echo "❌ Файл карты не найден: $PBF_PATH"
            rm -f "$TRIGGER_FILE"
            sleep 5
            continue
        fi

        # Генерируем имя .osrm рядом с osm.pbf
        OSRM_FILE="${PBF_PATH%.osm.pbf}.osrm"

        echo "⚡ Триггер найден! Обрабатываю карту: $PBF_PATH"

        # Останавливаем текущий osrm-routed если запущен
        if [ $CURRENT_PID -ne 0 ]; then
            echo "🛑 Останавливаем osrm-routed (PID=$CURRENT_PID)"
            kill $CURRENT_PID || true
            wait $CURRENT_PID || true
        fi

        # Препроцессинг карты для MLD
        echo "🛠 Препроцессинг карты для MLD..."
        osrm-extract -p /opt/car.lua "$PBF_PATH"
        osrm-partition "$OSRM_FILE"
        osrm-customize "$OSRM_FILE"
        echo "✅ Препроцессинг завершён: $OSRM_FILE"

        # Удаляем триггер
        rm -f "$TRIGGER_FILE"

        # Запускаем osrm-routed с MLD
        echo "▶ Запускаем osrm-routed (MLD)"
        osrm-routed --algorithm mld "$OSRM_FILE" &
        CURRENT_PID=$!
    fi
    sleep 5
done
