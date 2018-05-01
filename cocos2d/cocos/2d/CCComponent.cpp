/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "2d/CCComponent.h"


NS_CC_BEGIN

CCComponent::CCComponent(void)
: _owner(nullptr)
, _enabled(true)
{
#if CC_ENABLE_SCRIPT_BINDING
    ScriptEngineProtocol* engine = ScriptEngineManager::getInstance()->getScriptEngine();
    _scriptType = engine != nullptr ? engine->getScriptType() : kScriptTypeNone;
#endif
}

CCComponent::~CCComponent(void)
{
}

bool CCComponent::init()
{
    return true;
}

#if CC_ENABLE_SCRIPT_BINDING

static bool sendComponentEventToJS(CCComponent* node, int action)
{
    auto scriptEngine = ScriptEngineManager::getInstance()->getScriptEngine();
    
    if (scriptEngine->isCalledFromScript())
    {
        scriptEngine->setCalledFromScript(false);
    }
    else
    {
        BasicScriptData data(node,(void*)&action);
        ScriptEvent scriptEvent(kComponentEvent,(void*)&data);
        if (scriptEngine->sendEvent(&scriptEvent))
            return true;
    }
    
    return false;
}

#endif

void CCComponent::onEnter()
{
#if CC_ENABLE_SCRIPT_BINDING
    if (_scriptType == kScriptTypeJavascript)
    {
        if (sendComponentEventToJS(this, kComponentOnEnter))
            return;
    }
#endif
}

void CCComponent::onExit()
{
#if CC_ENABLE_SCRIPT_BINDING
    if (_scriptType == kScriptTypeJavascript)
    {
        if (sendComponentEventToJS(this, kComponentOnExit))
            return;
    }
#endif
}

void CCComponent::update(float delta)
{
#if CC_ENABLE_SCRIPT_BINDING
    if (_scriptType == kScriptTypeJavascript)
    {
        if (sendComponentEventToJS(this, kComponentOnUpdate))
            return;
    }
#endif
}

bool CCComponent::serialize(void *ar)
{
    return true;
}

CCComponent* CCComponent::create(void)
{
    CCComponent * ret = new (std::nothrow) CCComponent();
    if (ret != nullptr && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

const std::string& CCComponent::getName() const
{
    return _name;
}

void CCComponent::setName(const std::string& name)
{
    _name = name;
}

Node* CCComponent::getOwner() const
{
    return _owner;
}

void CCComponent::setOwner(Node *owner)
{
    _owner = owner;
}

bool CCComponent::isEnabled() const
{
    return _enabled;
}

void CCComponent::setEnabled(bool b)
{
    _enabled = b;
}

NS_CC_END
