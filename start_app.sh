#!/usr/bin/env bash
set -e

echo "Waiting for postgres..."
for i in {1..30}; do
  if pg_isready -h "${DB_HOST:-db}" -p "${DB_PORT:-5432}" -U "${DB_USER:-postgres}" >/dev/null 2>&1; then
    echo "Postgres is ready"
    break
  fi
  echo "Waiting... ($i)"
  sleep 1
done

echo "Starting application..."
cd /app/build
./app