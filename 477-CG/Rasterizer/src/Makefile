all: rasterizer
	
rasterizer:
	g++ -O3 *.cpp -o rasterizer -std=c++11

clean:
	rm rasterizer

test_clipped:
	./rasterizer inputs_outputs/clipping_example/empty_box_clipped.xml

test_horse_mug:
	./rasterizer inputs_outputs/culling_disabled_inputs/horse_and_mug.xml

test_germany_culling:
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_germany.xml

test_culling_enabled:	
	./rasterizer inputs_outputs/culling_enabled_inputs/empty_box.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/filled_box.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_brazil.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_czechia.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_czechia_alternative.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_eu.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_eu_alternative.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_germany.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_iceland.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_turkey.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/flag_turkey_alternative.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/horse_and_mug.xml
	./rasterizer inputs_outputs/culling_enabled_inputs/sample.xml

test_culling_disabled:
	./rasterizer inputs_outputs/culling_disabled_inputs/empty_box.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/filled_box.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_brazil.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_czechia.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_czechia_alternative.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_eu.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_eu_alternative.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_germany.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_turkey.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/flag_turkey_alternative.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/horse_and_mug.xml
	./rasterizer inputs_outputs/culling_disabled_inputs/sample.xml