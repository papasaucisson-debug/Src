#pragma once

class c_econ_item_view;

namespace econ_item_attribute_manager {
	void create(c_econ_item_view* item, int paint_kit, float wear, int seed);
	void remove(c_econ_item_view* item);
}
