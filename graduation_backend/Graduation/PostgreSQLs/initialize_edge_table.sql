INSERT INTO edges (id, the_geom, one_way, "name") SELECT osm_id, way, oneway, "name" FROM planet_osm_line WHERE highway IS NOT null AND railway IS null;

SELECT pgr_createTopology('edges', 0.000001);

UPDATE edges SET 
	x1 = st_x(st_startpoint(the_geom)),
	y1 = st_y(st_startpoint(the_geom)),
	x2 = st_x(st_endpoint(the_geom)),
	y2 = st_y(st_endpoint(the_geom)),
	cost_len = ST_length_Spheroid(the_geom, 'SPHEROID["GRS_1980",6378137,298.25728]'),
	rcost_len = st_length_spheroid(the_geom, 'SPHEROID["GRS_1980", 6378137, 298.25728]'),
	len_km = st_length_spheroid(the_geom, 'SPHEROID["WGS84",6378137,298.25728]') / 1000.0,
	len_miles = st_length_spheroid(the_geom, 'SPHEROID["WGS84",6378137,298.25728]') / 1000.0 * 0.6213712,

	speed_mph = CASE WHEN fcc='A10' THEN 65
			WHEN fcc='A15' THEN 65
			WHEN fcc='A20' THEN 55
			WHEN fcc='A25' THEN 55
			WHEN fcc='A30' THEN 45
			WHEN fcc='A35' THEN 45
			WHEN fcc='A40' THEN 35
			WHEN fcc='A45' THEN 35
			WHEN fcc='A50' THEN 25
			WHEN fcc='A60' THEN 25
			WHEN fcc='A61' THEN 25
			WHEN fcc='A62' THEN 25
			WHEN fcc='A64' THEN 25
			WHEN fcc='A70' THEN 15
			WHEN fcc='A69' THEN 10
			ELSE null END,
	speed_kmh = CASE WHEN fcc='A10' THEN 104
			WHEN fcc='A15' THEN 104
			WHEN fcc='A20' THEN 88
			WHEN fcc='A25' THEN 88
			WHEN fcc='A30' THEN 72
			WHEN fcc='A35' THEN 72
			WHEN fcc='A40' THEN 56
			WHEN fcc='A45' THEN 56
			WHEN fcc='A50' THEN 40
			WHEN fcc='A60' THEN 50
			WHEN fcc='A61' THEN 40
			WHEN fcc='A62' THEN 40
			WHEN fcc='A64' THEN 40
			WHEN fcc='A70' THEN 25
			WHEN fcc='A69' THEN 15
			ELSE null END;


