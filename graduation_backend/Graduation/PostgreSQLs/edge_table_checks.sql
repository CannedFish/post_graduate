SELECT pgr_analyzeGraph('edges', 0.000001);

SELECT pgr_analyzeOneway('edges',
	ARRAY['', 'B', 'TF'],
	ARRAY['', 'B', 'FT'],
	ARRAY['', 'B', 'FT'],
	ARRAY['', 'B', 'TF'],
	oneway:='one_way');