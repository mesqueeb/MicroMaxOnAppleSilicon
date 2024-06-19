#ifndef BOARD_COORD_HPP
#define BOARD_COORD_HPP

#define FLIP_COORD(x) 7 - x
#define CONVERT_FLIP_XY(x, y) !x ? y : 7 - y
#define CONVERT_FLIP(x) !self.isFlipped ? x : 7 - x

typedef long BoardCoord;

#endif