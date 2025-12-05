#!/usr/bin/env bash
set -e

TRIGGER_FILE="/app/maps/update.trigger"
CURRENT_PID=0

echo "Watcher (MLD) started. Wait for trigger: $TRIGGER_FILE"

while true; do
    if [ -f "$TRIGGER_FILE" ]; then
        # Read full path to osm.pbf
        PBF_PATH=$(cat "$TRIGGER_FILE" | tr -d '[:space:]')

        if [ -z "$PBF_PATH" ]; then
            echo "Trigger is empty, you need add full path .osm.pbf"
            rm -f "$TRIGGER_FILE"
            sleep 5
            continue
        fi

        if [ ! -f "$PBF_PATH" ]; then
            echo "Map file not found: $PBF_PATH"
            rm -f "$TRIGGER_FILE"
            sleep 5
            continue
        fi

        # Generate name .osrm by osm.pbf
        OSRM_FILE="${PBF_PATH%.osm.pbf}.osrm"

        echo "⚡ Trigger found! Procces the map: $PBF_PATH"

        # Stop current osrm-routed if is active 
        if [ $CURRENT_PID -ne 0 ]; then
            echo "Stop osrm-routed (PID=$CURRENT_PID)"
            kill $CURRENT_PID || true
            wait $CURRENT_PID || true
        fi

        # Map proccesing for MLD
        echo "Map preproccesing for MLD..."
        osrm-extract -p /opt/foot.lua "$PBF_PATH"
        osrm-partition "$OSRM_FILE"
        osrm-customize "$OSRM_FILE"
        echo "Preproccesing completed: $OSRM_FILE"

        # Delete trigger
        rm -f "$TRIGGER_FILE"

        # Start osrm-routed с MLD
        echo "▶ Start osrm-routed (MLD)"
        osrm-routed --algorithm mld "$OSRM_FILE" &
        CURRENT_PID=$!
    fi
    sleep 5
done
