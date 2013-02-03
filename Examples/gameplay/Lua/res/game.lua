-- This lua script file represents a lua implementation translation of sample00-mesh with a box instead of a duck.

function fillSphere(x, y, z, size, colour, paintOnly)
	local sizeSquared = size * size;
	for iZ=z-size, z+size do
		for iY=y-size, y+size do
			for iX=x-size, x+size do
				local xDist = x - iX
				local yDist = y - iY
				local zDist = z - iZ
				local distSquared = xDist * xDist + yDist * yDist + zDist * zDist;
				if distSquared < sizeSquared then
					if paintOnly then
						local currentColour = _colouredCubesVolume:getVoxel(iX, iY, iZ)
						if currentColour:z() > 0.001 then
							_colouredCubesVolume:setVoxel(iX, iY, iZ, colour, false)
						end
					else
						_colouredCubesVolume:setVoxel(iX, iY, iZ, colour, false)
					end
				end
			end
		end
	end
	_colouredCubesVolume:markRegionAsModified(x-size, y-size, z-size, x+size, y+size, z+size)
end

function initialize()
    -- Display splash screen for at least 1 second.
    -- ScreenDisplayer.start("drawSplash", 1000)

	wPressed = false
	sPressed = false
	aPressed = false
	dPressed = false

    leftMousePressed = false
	rightMousePressed = false
    _touchX = 0
	_touchY = 0

	_colouredCubesVolume = GameplayColouredCubesVolume.create(0, 0, 0, 127, 31, 127, 32, 16)
	GameplayVolumeSerialisation.gameplayLoadData("res/level2.vol", _colouredCubesVolume)

	--_colouredCubesVolume = GameplayColouredCubesVolume.create(0, 0, 0, 255, 255, 255, 32, 32)
	--GameplayVolumeSerialisation.gameplayLoadData("res/output.vol", _colouredCubesVolume)

    -- Load font
    _font = Font.create("res/arial40.gpb")

    -- Load mesh/scene from file
    --local bundle = Bundle.create("res/box.gpb")
    --_scene = bundle:loadScene()

	_scene = Scene.create()

	

    -- Create the paint brush
	local meshBundle = Bundle.create("res/Icosphere3.gpb")
	local mesh = meshBundle:loadMesh("Sphere_001")
	local model = Model.create(mesh)
	model:setMaterial("res/Icosphere3.material")

	_modelNode = Node.create()
	_modelNode:setModel(model)
	_scene:addNode(_modelNode)

    -- Create the light node
	local lightDirection = Vector3.new(0.01, 0.01, 0.01)
	_light = Light.createDirectional(lightDirection)
	lightNode = Node.create()
	lightNode:setLight(_light)
	lightNode:setTranslation(0.0, 100.0, 0.0)
	lightNode:rotateX(-1.57) -- Point light down
	_scene:addNode(lightNode)

    -- Bind the light node's direction into the box material.
    _modelNode:getModel():getMaterial():getParameter("u_lightDirection"):bindValue(lightNode, "&Node::getForwardVectorView")

    -- Update the aspect ratio for our scene's camera to match the current device resolution
    local game = Game.getInstance()

	--Camera model based on http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Creating+a+simple+first-person+camera+system
	_cameraPositionNode = _scene:addNode()
	_cameraYawNode = Node.create()
	_cameraPositionNode:addChild(_cameraYawNode)
	_cameraPitchNode = Node.create()
	_cameraYawNode:addChild(_cameraPitchNode)
	_cameraNode = Node.create()
	_cameraPitchNode:addChild(_cameraNode)

	local camera = Camera.createPerspective(60.0, 1.0, 0.5, 2000.0)
	camera:setAspectRatio(game:getWidth() / game:getHeight())
	_cameraNode:setCamera(camera)
	_scene:setActiveCamera(camera)
	_cameraPositionNode:setTranslation(0.0, 0.0, 100.0)

	local dummyValue = 42
	_scene:addNode(_colouredCubesVolume:getRootNodeForLua(dummyValue))

    -- ScreenDisplayer.finish()
end

function update(elapsedTime)
	local forwardVector = _cameraNode:getForwardVectorWorld()
	local rightVector = _cameraNode:getRightVectorWorld()

	local speed = 0.1
	local distance = elapsedTime * speed

	if(wPressed) then
		forwardVector:scale(distance)
		_cameraPositionNode:translate(forwardVector)
	elseif(sPressed) then
		forwardVector:scale(-distance)
		_cameraPositionNode:translate(forwardVector)
	elseif(aPressed) then
		rightVector:scale(-distance)
		_cameraPositionNode:translate(rightVector)
	elseif(dPressed) then
		rightVector:scale(distance)
		_cameraPositionNode:translate(rightVector)
	end

	local viewPos = _cameraPositionNode:getTranslationWorld()
	_colouredCubesVolume:performUpdate(viewPos, 0.25)

	if(_colouredCubesVolume) then
		ray = Ray.new();
		_cameraNode:getCamera():pickRay(Game.getInstance():getViewport(), _touchX, _touchY, ray);

		dir = ray:getDirection()
		dir:scale(200.0)
		ray:setDirection(dir)

		intersection = Vector3.new()
		if(GameplayRaycasting.gameplayRaycast(_colouredCubesVolume, ray, 200.0, intersection)) then
			_modelNode:setTranslation(intersection)
		end

		if(leftMousePressed) then
			--_colouredCubesVolume:setVoxel(intersection:x(), intersection:y(), intersection:z(), 0.99, 0.0, 0.0)
			local colour = Vector4.new(0.99, 0.0, 0.0, 1.0)
			fillSphere(intersection:x(), intersection:y(), intersection:z(), _modelNode:getScaleX(), colour, true)
		end
	end


end

-- Avoid allocating new objects every frame.
textColor = Vector4.new(0, 0.5, 1, 1)

function render(elapsedTime)
    -- Clear the color and depth buffers.
    Game.getInstance():clear(Game.CLEAR_COLOR_DEPTH, Vector4.zero(), 1.0, 0)

    -- Visit all the nodes in the scene, drawing the models/mesh.
    _scene:visit("drawScene")

    -- Draw the fps.
    local buffer = string.format("%u", Game.getInstance():getFrameRate())
    _font:start()
    _font:drawText(buffer, 5, 1, textColor, _font:getSize())
    _font:finish()
end

function finalize()
    _font = nil
    _scene = nil
end

function drawScene(node)
	-- Skip note if the flag is set to false.
	local nodeTag = tostring(node:getTag("RenderThisNode"))
	if(nodeTag == "f") then
	 	return true
	end

	local model = node:getModel()
    if model then
		if (model:getMaterial()) then
			if (model:getMaterial():getParameter("u_lightColor")) then
				model:getMaterial():getParameter("u_lightColor"):setValue(_light:getColor());
			end
			if (model:getMaterial():getParameter("u_lightDirection")) then
				model:getMaterial():getParameter("u_lightDirection"):setValue(lightNode:getForwardVectorWorld());
			end
		end
        model:draw()
    end

    return true
end

function drawSplash()
    local game = Game.getInstance()
    game:clear(Game.CLEAR_COLOR_DEPTH, 0, 0, 0, 1, 1.0, 0)
    local batch = SpriteBatch.create("res/logo_powered_white.png")
    batch:start()
    batch:draw(game:getWidth() * 0.5, game:getHeight() * 0.5, 0.0, 512.0, 512.0, 0.0, 1.0, 1.0, 0.0, Vector4.one(), true)
    batch:finish()
end

function keyEvent(evt, key)
    if evt == Keyboard.KEY_PRESS then
        if key == Keyboard.KEY_ESCAPE then
            Game.getInstance():exit()
		elseif key == Keyboard.KEY_W then
			wPressed = true;
		elseif key == Keyboard.KEY_A then
			aPressed = true;
		elseif key == Keyboard.KEY_S then
			sPressed = true;
		elseif key == Keyboard.KEY_D then
			dPressed = true;
        end
	elseif evt == Keyboard.KEY_RELEASE then
		if key == Keyboard.KEY_W then
			wPressed = false;
		elseif key == Keyboard.KEY_A then
			aPressed = false;
		elseif key == Keyboard.KEY_S then
			sPressed = false;
		elseif key == Keyboard.KEY_D then
			dPressed = false;
        end

    end
end

function mouseEvent(evt, x, y, wheelDelta)
    if evt == Mouse.MOUSE_PRESS_LEFT_BUTTON then
        leftMousePressed = true
    elseif evt == Mouse.MOUSE_RELEASE_LEFT_BUTTON then
        leftMousePressed = false
	elseif evt == Mouse.MOUSE_PRESS_RIGHT_BUTTON then
        rightMousePressed = true
    elseif evt == Mouse.MOUSE_RELEASE_RIGHT_BUTTON then
        rightMousePressed = false
    elseif evt == Mouse.MOUSE_MOVE  then
        local deltaX = x - _touchX
		local deltaY = y - _touchY
        _touchX = x
		_touchY = y
		if(rightMousePressed) then
			_cameraYawNode:rotateY(math.rad(deltaX * -0.5))
			_cameraPitchNode:rotateX(math.rad(deltaY * -0.5))
		end
    end

	-- If the wheelDelta is positive then enlarge the brush a bit for each tick
	local perTickScale = 1.1	
	for i=1, wheelDelta do
		_modelNode:scale(perTickScale, perTickScale, perTickScale)
	end

	-- If the wheelDelta is positive then shrink the brush a bit for each tick
	local invPerClickScale = 1.0 / perTickScale
	for i=-1, wheelDelta, -1 do
		-- Prevent the brush from being shrunk to nothing.
		if math.min(math.min(_modelNode:getScaleX(), _modelNode:getScaleY()), _modelNode:getScaleZ()) > 0.1 then
			_modelNode:scale(invPerClickScale, invPerClickScale, invPerClickScale)
		end
	end

	return true;  
end