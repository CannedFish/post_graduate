DROP TABLE IF EXISTS edges;

CREATE TABLE edges (
	id bigint,
        source bigint,
        target bigint,
	the_geom geometry
);

ALTER TABLE edges
	ADD COLUMN "name" text,
	ADD COLUMN one_way text,
	ADD COLUMN cost_len double precision,
	ADD COLUMN cost_time double precision,
	ADD COLUMN rcost_len double precision,
	ADD COLUMN rcost_time double precision,
	ADD COLUMN x1 double precision,
	ADD COLUMN y1 double precision,
	ADD COLUMN x2 double precision,
	ADD COLUMN y2 double precision,
	ADD COLUMN to_cost double precision,
	ADD COLUMN rule text,
	ADD COLUMN isolated integer,
	ADD COLUMN len_km double precision,
	ADD COLUMN len_miles double precision,
	ADD COLUMN fcc text,
	ADD COLUMN speed_mph integer,
	ADD COLUMN speed_kmh integer;
