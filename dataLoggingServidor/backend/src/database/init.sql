-- criar tabela
CREATE TABLE IF NOT EXISTS sensores (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(255) NOT NULL,
    sensor VARCHAR(30) NOT NULL,
    location VARCHAR(30) NOT NULL,
    value1 VARCHAR(10),
    value2 VARCHAR(10),
    reading_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);