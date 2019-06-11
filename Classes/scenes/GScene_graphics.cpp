//
//  GScene_map.cpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GScene.hpp"
#include "enum.h"
#include "menu.h"
#include "util.h"

void GScene::renderSpace()
{
	spaceRender->setVisible(display != displayMode::lightmap);
	lightmapRender->setVisible(display != displayMode::base);
	lightmapBackground->setVisible(display == displayMode::lightmap);

	graphicsContext->update();

	redrawLightmap();
}

void GScene::redrawLightmap()
{
	lightmapDrawNode->clear();

	CCRect cameraPix = getCameraArea().toPixelspace();

	if (display == displayMode::lightmap) {
		lightmapDrawNode->drawSolidRect(
			cameraPix.getLowerCorner(),
			cameraPix.getUpperCorner(),
			Color4F::BLACK
		);
	}
}

void GScene::cycleDisplayMode()
{
	enum_increment(displayMode, display);
	if (display == displayMode::end) {
		display = displayMode::begin;
	}
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer, BlendFunc blend)
{
	RenderTexture* rt = initRenderTexture(sceneLayer);
	rt->getSprite()->setBlendFunc(blend);
	return rt;
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer)
{
	unsigned int width = app::params.width;
	unsigned int height = app::params.height;

	RenderTexture* rt = RenderTexture::create(width, height);
	rt->setPosition(width / 2.0f, height / 2.0f);
	addChild(rt, to_int(sceneLayer));
	rt->addChild(getLayer(sceneLayer));
	rt->setClearColor(Color4F(0.0f, 0.0f, 0.0f, 0.0f));
	rt->setAutoDraw(true);
	rt->setClearFlags(GL_COLOR_BUFFER_BIT);
	return rt;
}

void GScene::setColorFilter(const Color4F& color)
{
	unsigned int width = app::params.width;
	unsigned int height = app::params.height;

	colorFilterDraw->clear();
	colorFilterDraw->drawSolidRect(-1.0f*Vec2(width/2, height/2), Vec2(width/2, height/2), color);
}
