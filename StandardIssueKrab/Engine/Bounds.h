#pragma once

template<class T, class ComparableT = T>
struct Bounds {
	ComparableT min, max;
};