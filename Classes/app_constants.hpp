//
//  app_constants.hpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#ifndef app_constants_hpp
#define app_constants_hpp

namespace app
{
	constexpr unsigned int baseWidth = 1600;
	constexpr unsigned int baseHeight = 1000;

	constexpr int pixelsPerTile = 128;
	constexpr float tilesPerPixel = 1.0f / pixelsPerTile;
	constexpr float viewWidth = 22.5f;

	constexpr double Gaccel = 9.806;
}   

#define float_pi boost::math::constants::pi<SpaceFloat>()
constexpr SpaceFloat float_2pi = float_pi*2.0;
constexpr SpaceFloat pi_inv = 1.0 / float_pi;
constexpr SpaceFloat atan_limit_sq = 1e-6;

#endif
