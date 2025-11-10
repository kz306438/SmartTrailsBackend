-- ============================================================
-- SmartTrails Database Initialization Script
-- PostgreSQL 17 + PostGIS
-- File: init.sql
-- ============================================================

-- ============================================================
-- 0. CLEANUP (drop all existing objects in correct order)
-- ============================================================

DROP TABLE IF EXISTS route_poi CASCADE;
DROP TABLE IF EXISTS routes CASCADE;
DROP TABLE IF EXISTS user_preferences CASCADE;
DROP TABLE IF EXISTS poi CASCADE;
DROP TABLE IF EXISTS poi_types CASCADE;
DROP TABLE IF EXISTS map_sources CASCADE;
DROP TABLE IF EXISTS users CASCADE;
DROP TYPE IF EXISTS user_role CASCADE;

-- ============================================================
-- 1. EXTENSIONS
-- ============================================================

CREATE EXTENSION IF NOT EXISTS postgis;

-- ============================================================
-- 2. ENUM TYPES
-- ============================================================

CREATE TYPE user_role AS ENUM ('user', 'admin');

-- ============================================================
-- 3. USERS
-- ============================================================

CREATE TABLE users (
    id              SERIAL PRIMARY KEY,
    username        VARCHAR(50) UNIQUE NOT NULL,
    email           VARCHAR(100) UNIQUE NOT NULL,
    password_hash   TEXT NOT NULL,
    role            user_role DEFAULT 'user' NOT NULL,
    created_at      TIMESTAMPTZ DEFAULT NOW()
);

-- ============================================================
-- 4. MAP SOURCES
-- ============================================================

CREATE TABLE map_sources (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(100) NOT NULL,
    path            TEXT NOT NULL,
    is_active       BOOLEAN DEFAULT FALSE,
    created_at      TIMESTAMPTZ DEFAULT NOW()
);

-- ============================================================
-- 5. POI TYPES
-- ============================================================

CREATE TABLE poi_types (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(50) UNIQUE NOT NULL
);

-- ============================================================
-- 6. POI (Places of Interest)
-- ============================================================

CREATE TABLE poi (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(100) NOT NULL,
    city            VARCHAR(100),
    type_id         INT REFERENCES poi_types(id) ON DELETE SET NULL,
    coordinates     GEOMETRY(Point, 4326) NOT NULL,
    description     TEXT,
    map_source_id   INT REFERENCES map_sources(id) ON DELETE SET NULL,
    created_at      TIMESTAMPTZ DEFAULT NOW()
);

-- Spatial index for fast geo queries
CREATE INDEX idx_poi_geom ON poi USING GIST (coordinates);

-- ============================================================
-- 7. USER PREFERENCES
-- ============================================================

CREATE TABLE user_preferences (
    id                  SERIAL PRIMARY KEY,
    user_id             INT REFERENCES users(id) ON DELETE CASCADE,
    preferred_distance_km NUMERIC(6,2),
    poi_type_ids        INT[],
    updated_at          TIMESTAMPTZ DEFAULT NOW()
);

-- ============================================================
-- 8. ROUTES
-- ============================================================

CREATE TABLE routes (
    id              SERIAL PRIMARY KEY,
    user_id         INT REFERENCES users(id) ON DELETE CASCADE,
    preference_id   INT REFERENCES user_preferences(id) ON DELETE SET NULL,
    name            VARCHAR(100) NOT NULL,
    start_point     GEOMETRY(Point, 4326) NOT NULL,
    route_line      GEOMETRY(LineString, 4326) NOT NULL,
    distance_km     NUMERIC(7,2),
    map_source_id   INT REFERENCES map_sources(id) ON DELETE SET NULL,
    created_at      TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_routes_geom ON routes USING GIST (route_line);
CREATE INDEX idx_routes_start ON routes USING GIST (start_point);

-- ============================================================
-- 9. ROUTE <-> POI MAPPING
-- ============================================================

CREATE TABLE route_poi (
    route_id    INT REFERENCES routes(id) ON DELETE CASCADE,
    poi_id      INT REFERENCES poi(id) ON DELETE CASCADE,
    PRIMARY KEY (route_id, poi_id)
);

-- ============================================================
-- 10. TRIGGERS AND CONSTRAINTS
-- ============================================================

-- Only one active map source at a time
CREATE OR REPLACE FUNCTION enforce_single_active_map()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.is_active THEN
        UPDATE map_sources SET is_active = FALSE WHERE id <> NEW.id;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_single_active_map
BEFORE INSERT OR UPDATE ON map_sources
FOR EACH ROW EXECUTE FUNCTION enforce_single_active_map();

-- ============================================================
-- 11. INITIAL DATA (OPTIONAL)
-- ============================================================

INSERT INTO poi_types (name) VALUES
    ('cafe'),
    ('restaurant'),
    ('park'),
    ('museum'),
    ('landmark')
ON CONFLICT DO NOTHING;

-- ============================================================
-- END OF INIT SCRIPT
-- ============================================================

-- COMMENT ON DATABASE current_database() IS 'SmartTrails database schema with PostGIS support (init.sql)';
