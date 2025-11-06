-- ===========================
-- SmartTrails Database Init
-- PostgreSQL 17.x
-- ===========================

-- Drop existing tables if they exist (for dev reset)
DROP TABLE IF EXISTS route_places CASCADE;
DROP TABLE IF EXISTS route_points CASCADE;
DROP TABLE IF EXISTS routes CASCADE;
DROP TABLE IF EXISTS user_preferences CASCADE;
DROP TABLE IF EXISTS places_of_interest CASCADE;
DROP TABLE IF EXISTS map_sources CASCADE;
DROP TABLE IF EXISTS system_metrics CASCADE;
DROP TABLE IF EXISTS users CASCADE;

-- =======================================
-- USERS
-- =======================================
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(10) NOT NULL CHECK (role IN ('user', 'admin')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_username ON users(username);

-- -- =======================================
-- -- USER PREFERENCES
-- -- =======================================
-- CREATE TABLE user_preferences (
--     id SERIAL PRIMARY KEY,
--     user_id INT REFERENCES users(id) ON DELETE CASCADE,
--     preferred_terrain VARCHAR(20) CHECK (preferred_terrain IN ('city', 'park', 'forest')),
--     preferred_distance_km NUMERIC(5,2) CHECK (preferred_distance_km > 0),
--     include_places BOOLEAN DEFAULT TRUE,
--     created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
-- );

-- -- =======================================
-- -- ROUTES
-- -- =======================================
-- CREATE TABLE routes (
--     id SERIAL PRIMARY KEY,
--     user_id INT REFERENCES users(id) ON DELETE SET NULL,
--     name VARCHAR(100),
--     description TEXT,
--     start_lat DOUBLE PRECISION NOT NULL,
--     start_lng DOUBLE PRECISION NOT NULL,
--     distance_km NUMERIC(6,2),
--     terrain VARCHAR(20) CHECK (terrain IN ('city', 'park', 'forest')),
--     created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
--     is_saved BOOLEAN DEFAULT TRUE
-- );

-- CREATE INDEX idx_routes_user_id ON routes(user_id);

-- -- =======================================
-- -- ROUTE POINTS
-- -- =======================================
-- CREATE TABLE route_points (
--     id SERIAL PRIMARY KEY,
--     route_id INT REFERENCES routes(id) ON DELETE CASCADE,
--     point_order INT NOT NULL,
--     latitude DOUBLE PRECISION NOT NULL,
--     longitude DOUBLE PRECISION NOT NULL
-- );

-- CREATE INDEX idx_route_points_route_id ON route_points(route_id);

-- -- =======================================
-- -- PLACES OF INTEREST
-- -- =======================================
-- CREATE TABLE places_of_interest (
--     id SERIAL PRIMARY KEY,
--     name VARCHAR(100) NOT NULL,
--     type VARCHAR(50) CHECK (type IN ('cafe', 'restaurant', 'landmark', 'museum', 'park')),
--     latitude DOUBLE PRECISION NOT NULL,
--     longitude DOUBLE PRECISION NOT NULL,
--     description TEXT
-- );

-- CREATE INDEX idx_places_lat_lng ON places_of_interest(latitude, longitude);

-- -- =======================================
-- -- ROUTE_PLACES (many-to-many)
-- -- =======================================
-- CREATE TABLE route_places (
--     route_id INT REFERENCES routes(id) ON DELETE CASCADE,
--     place_id INT REFERENCES places_of_interest(id) ON DELETE CASCADE,
--     PRIMARY KEY (route_id, place_id)
-- );

-- -- =======================================
-- -- MAP SOURCES (admin controlled)
-- -- =======================================
-- CREATE TABLE map_sources (
--     id SERIAL PRIMARY KEY,
--     name VARCHAR(100) NOT NULL,
--     url TEXT NOT NULL,
--     is_active BOOLEAN DEFAULT TRUE,
--     updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
-- );

-- -- =======================================
-- -- SYSTEM METRICS (for admin monitoring)
-- -- =======================================
-- CREATE TABLE system_metrics (
--     id SERIAL PRIMARY KEY,
--     cpu_usage NUMERIC(5,2),
--     memory_usage NUMERIC(5,2),
--     active_users INT,
--     timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
-- );

-- -- =======================================
-- -- SAMPLE DATA
-- -- =======================================

-- -- Admin and user accounts (passwords are bcrypt placeholders)
-- INSERT INTO users (username, email, password_hash, role)
-- VALUES
-- ('admin', 'admin@smarttrails.io', '$2a$10$exampleadminhash', 'admin'),
-- ('user1', 'user1@example.com', '$2a$10$exampleuserhash', 'user');

-- -- Default user preferences
-- INSERT INTO user_preferences (user_id, preferred_terrain, preferred_distance_km, include_places)
-- VALUES
-- (2, 'park', 5.00, TRUE);

-- -- Default POI
-- INSERT INTO places_of_interest (name, type, latitude, longitude, description)
-- VALUES
-- ('Central Park Cafe', 'cafe', 40.785091, -73.968285, 'Cozy cafe inside the park'),
-- ('City Museum', 'museum', 40.779437, -73.963244, 'Famous historical museum'),
-- ('Old Fountain', 'landmark', 40.782865, -73.965355, 'Iconic city landmark'),
-- ('Riverside Park', 'park', 40.800678, -73.970833, 'Scenic park along the river');

-- -- Example route
-- INSERT INTO routes (user_id, name, description, start_lat, start_lng, distance_km, terrain)
-- VALUES
-- (2, 'Morning Run in the Park', 'Scenic 5km run through the park with coffee stop.', 40.785091, -73.968285, 5.00, 'park');

-- -- Route points
-- INSERT INTO route_points (route_id, point_order, latitude, longitude)
-- VALUES
-- (1, 1, 40.785091, -73.968285),
-- (1, 2, 40.782865, -73.965355),
-- (1, 3, 40.779437, -73.963244),
-- (1, 4, 40.780678, -73.970833);

-- -- Link POIs to the route
-- INSERT INTO route_places (route_id, place_id)
-- VALUES
-- (1, 1),
-- (1, 2),
-- (1, 3);

-- -- Map sources
-- INSERT INTO map_sources (name, url, is_active)
-- VALUES
-- ('OpenStreetMap', 'https://tile.openstreetmap.org/{z}/{x}/{y}.png', TRUE);

-- -- System metric sample
-- INSERT INTO system_metrics (cpu_usage, memory_usage, active_users)
-- VALUES
-- (15.2, 42.8, 7);

