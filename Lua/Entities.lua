--[[
	CONTRIBUTOR: straykevin
	DATE: January, 2023
	PURPOSE: This is a module which enables the game to have persistent data amongst server sessions. 
	When all player's leave the game, it will save all entities left under workspace.

]]

--[[ ROBLOX API DEPENDENCIES]]--
local DataStoreService = game:GetService("DataStoreService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local TeleportService = game:GetService("TeleportService")
local ServerScriptService = game:GetService("ServerScriptService")
local Players = game:GetService("Players")

--[[REQUIRED MODULE SCRIPTS]]--
local PickUpFunctions = require(script.PickUpFunctions)
local EntityFunctions = require(script.EntityFunctions)
local DataHandler = require(ServerScriptService.Libraries.DataHandler)
local Roles = require(ServerScriptService.ModuleScripts.Config.Roles)

--[[INITIALIZING KEY VARIABLES]]--
local dataStore = DataStoreService:GetGlobalDataStore()
local privateServerId = game.PrivateServerId
local savedServerId = dataStore:GetAsync("ReservedServerTEST_" .. game.PlaceId)
local savedEntityKey = "Serve_" .. string.sub(savedServerId, 1, #savedServerId / 2) .. "_Ent"

--[[KEEPS TESTING DATASTORE SEPERATE FROM LIVE DATASTORES]]--
if game:GetService("RunService"):IsStudio() then
	savedEntityKey = savedEntityKey .. "_TEST" 	
end

--[[ PRIVATE CONFIG VARIABLES]]--
local MAX_DROP_DISTANCE = 5
local MAIN_GROUP_ID = 6439623
local ADMIN_PRIVELEGES = 100

--[[ PRIVATE LOCAL FUNCTIONS]]--


--[[
	FUNCTION NAME: stringtocf
	PARAMETERS: string
	DESCRIPTION: Converts strings to CFrames.
	
	RETURNS: cframe
]]
local function stringtocf(str: string)
	return CFrame.new(table.unpack(str:gsub(" ",""):split(",")))
end


--[[
	FUNCTION NAME: setNetworkOwner
	PARAMETERS: Player, Instance, Boolean
	DESCRIPTION: Sets the network owner of an Instance's basepart children to the player.
	
	RETURNS: N/A
]]
local function setNetworkOwner(player, instance, toggle)
	
	-- if bool == true, then the player is requesting the action. Otherwise, we continue to ignore server-related actions...
	if toggle and instance:GetAttribute("Anchored") then return end
	for _, entity in pairs(instance:GetChildren()) do

		if entity:IsA("BasePart") then
			entity.Anchored = false
			entity:SetNetworkOwner(player)
		end

	end
end


local CurrEntities = {} -- Table Consistent of all active entities within the game


-- [[ Entity Object ]]
local Entity = {}
Entity.__index = Entity


--[[
	FUNCTION NAME: PlaceItemEvent [OnServerEvent]
	PARAMETERS: Player
	DESCRIPTION: Determines if there exists a player's tool on the player's character. If it exists and is elgibible to be dropped, place it within workspace. 
	The function essentially converts a present tool to a savable entity under workspace.
	
	RETURNS: N/A
]]
ReplicatedStorage.CoreRemotes.PlaceItemEvent.OnServerEvent:Connect(function(player)
	if player.Character then
		local tool = player.Character:FindFirstChildWhichIsA("Tool")
		
		if not tool then
			return
		elseif tool:GetAttribute("CanBeDropped") == false then
			return
		end
		
		local tCFrame
		
		local raycastParams = RaycastParams.new()
		raycastParams.FilterType = Enum.RaycastFilterType.Blacklist
		raycastParams.FilterDescendantsInstances = {player.Character}
		
		local direction = player.Character.Head.CFrame.LookVector * MAX_DROP_DISTANCE
		local raycastResult = workspace:Raycast(player.Character.Head.Position, direction, raycastParams)
		
		if raycastResult then
			tCFrame = CFrame.new(raycastResult.Position)
		else
			tCFrame = CFrame.new(player.Character.Head.Position + (direction))
		end
		 

		Entity.new(tool, tCFrame)
	end
end)


--[[
	FUNCTION NAME: On Server Shutdown
	PARAMETERS: N/A
	DESCRIPTION: Saves all applicable entities under workspace to the global datastore for the next time the server becomes lives.
	Saves values such as attributes and cframes.
	Also the place to be when determining save behavior for special entities.
	
	RETURNS: N/A
]]
game:BindToClose(function()
	local serialize = {}
	for instance, tbl in pairs(CurrEntities) do
		local success, response = pcall(function()
			local count = 0
			local key 

			repeat count += 1 key = tostring(instance.Name) .. "_" .. count until not serialize[key]



			-- saving attributes
			local val = instance:GetAttributes() or {}


			-- saving cframe
			val["_CF"] = tostring(instance.PrimaryPart.CFrame)


			if instance.Name == "Cable" then
				for _, child in pairs(instance:GetChildren()) do
					if child:IsA("BasePart") then
						val["_CF" .. child.Name] = tostring(instance.CFrame)
					end
				end
			end

			serialize[key] = val

		end)

		if not success then
			warn(response)
		end
	end

	dataStore:SetAsync(savedEntityKey, serialize)
end)


--[[
	FUNCTION NAME: On Server Shutdown
	PARAMETERS: N/A
	DESCRIPTION: Saves all applicable entities under workspace to the global datastore for the next time the server becomes lives.
	Saves values such as attributes and cframes.
	Also the place to be when determining save behavior for special entities.
	
	RETURNS: N/A
]]
ReplicatedStorage.CoreRemotes.DeleteEntityEvent.OnServerEvent:Connect(function(player, entity)
	Entity:DestroyEntity(player, entity)
end)


--[[
	FUNCTION NAME: RequestNetworkOwnership [Server-Side]
	PARAMETERS: Player, Instance
	DESCRIPTION: Gives access of the target entity. Prior to giving access, first determines whether player's control is legal.
	
	RETURNS: Boolean
]]
ReplicatedStorage.CoreRemotes.RequestNetworkOwnership.OnServerInvoke = function(player, entity)
	if CurrEntities[entity] then
		if entity:GetAttribute("Owner") == player.Name or player:GetRankInGroup(MAIN_GROUP_ID) >= ADMIN_PRIVELEGES then
			setNetworkOwner(player, entity)
			return true
		end
	end
	
	return false
end


--[[
	FUNCTION NAME: BuildEvent [On Server Event]
	PARAMETERS: Player, String, Instance
	DESCRIPTION: Manages player-related building behavior. Has several context
	
	RETURNS: N/A
]]
ReplicatedStorage.CoreRemotes.BuildEvent.OnServerEvent:Connect(function(player, context, entity)
	
	if CurrEntities[entity] then
		if context == "Anchor" then
			local anchored = entity:GetAttribute("Anchored")
			
			--Confusing
			if anchored then
				entity:SetAttribute("Anchored", not anchored)
			else
				entity:SetAttribute("Anchored", true)
			end

			for _, part in pairs(entity:GetChildren()) do
				if part:IsA("BasePart") then
					part.Anchored = entity:GetAttribute("Anchored")
				end
			end

		end
	end
	
end)


--[[
	FUNCTION NAME: DestroyEntity 
	PARAMETERS: Player, String, Instance
	DESCRIPTION: The only way to appropiately get rid of entities within the world.
	
	RETURNS: N/A
]]
function Entity:DestroyEntity(player: Player, entity: Instance)
	if CurrEntities[entity] then
		if entity:GetAttribute("Owner") == player.Name or player:GetRankInGroup(MAIN_GROUP_ID) >= ADMIN_PRIVELEGES then
			CurrEntities[entity]:Destroy()
		end
	end
end


--[[
	FUNCTION NAME: GetEntity 
	PARAMETERS: Instance
	DESCRIPTION: Returns the entity object related to the given instance. If no key matches, returns nil.
	
	RETURNS: A found entity, otherwise, nil.
]]
function Entity:GetEntity(instance: Instance)
	return CurrEntities[instance]
end

--[[
	FUNCTION NAME: GetAllInstancesOfOwner 
	PARAMETERS: string
	DESCRIPTION: Returns a list of all entities owned by player
	
	RETURNS: table containing instances
]]
function Entity:GetAllInstancesOfOwner(owner: string)
	local entityTable = {}
	for _, entity in pairs(CurrEntities) do
		if entity.Instance:GetAttribute("Owner") == owner then
			table.insert(entityTable, entity.Instance)
		end
	end
	
	return entityTable
end

--[[
	FUNCTION NAME: Constructor 
	PARAMETERS: Models/Parts/Tools, CFrame/Model, table, Player/String
	DESCRIPTION: Creates an entity using a template instance and various (optional) parameters.
	
	RETURNS: Entity Object
]]
function Entity.new(instance: Instance, cframe, currAttributes: table, owner) -- instance: Instance (tool, model, part), cframe: Last Known Saved Location
	local newEntity = {}
	setmetatable(newEntity, Entity)
		
	newEntity.Connections = {}
	
	local children = instance:GetChildren()
	local attributes = 	newEntity:InitAttributes(instance, owner, currAttributes)-- either used saved attributes or new attributes...
	

	-- Begins process of converting original instance into a savable 'entity')
	if instance:IsA("Tool") then
		
		local model = Instance.new("Model")

		for _, child in pairs(instance:GetChildren()) do

			if child:IsA("BasePart") then
				child.CanCollide = true
				child.Anchored = false

				if child.Name == "Handle" then
					model.PrimaryPart = child
				end
			elseif (child:IsA("LocalScript") or child:IsA("Script")) and (child.Name ~= "qPerfectionWeld" and child.Name ~= "Listener") then
				child.Enabled = false
			end
			
			child.Parent = model
		end
		
		newEntity.Instance = model
		
	elseif instance:IsA("BasePart") then
		
		local model = Instance.new("Model")
		instance.Parent = model
		model.PrimaryPart = instance
		newEntity.Instance = model
		
	elseif instance:IsA("Model") then
		
		newEntity.Instance = instance
		
	else
		error("Invalid Instance was submitted!")
	end
	
	-- At this point, the instance either got converted or was a model.
	for attribute, value in pairs(attributes) do
		if typeof(value) ~= "Instance" then
			newEntity.Instance:SetAttribute(attribute, value)
		end
		
	end
	
	-- Determines behavior of entity once spawned under workspace.
	local isTool = instance:IsA("Tool")
	local isMoney = instance:GetAttribute("Value")
	local isBook = instance:GetAttribute("ExpValue")
	
	local grip = isTool and instance.Grip
	
	local ActionText = instance:GetAttribute("ActionText")

	-- Applies Proximity Prompt Behavior based on previous context (344 - 351)
	if isTool then
		
		local ProximityPrompt = Instance.new("ProximityPrompt")
		ProximityPrompt.ObjectText = instance.Name
		ProximityPrompt.ActionText = ActionText or "Pick Up"
		ProximityPrompt.RequiresLineOfSight = false
		
		if PickUpFunctions[instance.Name] or isMoney then -- Behavior for direct collectables. (Such as EXP, Money, etc.)
			
			if isMoney then 
				
				ProximityPrompt.Triggered:Connect(function(player)
					if player.Character.Humanoid.Health <= 0 then return end
					
					PickUpFunctions["Cash"](player, newEntity)

					local newInstance = newEntity.Instance
					CurrEntities[newEntity.Instance] = nil
					newInstance:Destroy()
				end)
				
			elseif isBook then
				
				ProximityPrompt.Triggered:Connect(function(player)
					if player.Character.Humanoid.Health <= 0 then return end

					PickUpFunctions["Book"](player, newEntity)

					local newInstance = newEntity.Instance
					CurrEntities[newEntity.Instance] = nil
					newInstance:Destroy()
				end)
				
			else -- Default Interaction behavior for basic entities.
				
				ProximityPrompt.Triggered:Connect(function(player)
					if player.Character.Humanoid.Health <= 0 then return end
					
					PickUpFunctions[instance.Name](player, newEntity)

					local newInstance = newEntity.Instance
					CurrEntities[newEntity.Instance] = nil
					newInstance:Destroy()
				end)
				
			end

		else -- Default Behavior for Player Tools. (Equips tool to player's hand.)
			
			ProximityPrompt.Triggered:Connect(function(player)
				if player.Character.Humanoid.Health <= 0 then return end
				
				local tool = Instance.new("Tool")
				tool.Grip = grip
				tool.Name = newEntity.Instance.Name
				
				ProximityPrompt:Destroy()
				
				for _, child in pairs(newEntity.Instance:GetChildren()) do

					if child:IsA("BasePart") then
						child.CanCollide = false
						child.Anchored = false
					elseif child:IsA("LocalScript") or child:IsA("Script") then
						child.Enabled = true
					end

					child.Parent = tool
				end
			
				for attribute, value in pairs(attributes) do
					if typeof(value) ~= "Instance" then
						tool:SetAttribute(attribute, value)
					end
				end
				
				local foundTool = player.Character:FindFirstChildWhichIsA("Tool")
				
				if foundTool then
					foundTool.Parent = player.Backpack
				end
				
				tool.Parent = player.Character
				
				local newInstance = newEntity.Instance
				CurrEntities[newEntity.Instance] = nil
				newInstance:Destroy()
			end)
			
		end
		
		instance:Destroy()

		ProximityPrompt.Parent = newEntity.Instance.PrimaryPart
		newEntity.ProximityPrompt = ProximityPrompt
	end
	
	if attributes["Anchored"] then
		for _, part in pairs(newEntity.Instance:GetChildren()) do
			if part:IsA("BasePart") then
				part.Anchored = attributes["Anchored"]
			end
		end
	end
	
	newEntity.Instance.Parent = workspace
	
	if EntityFunctions[instance.Name] then
		EntityFunctions[instance.Name](newEntity, attributes["Level"], owner) -- Applies unqiue effects to entities. (Prints Money, Collect Money, etc.)
	end
	
	--[[ These lines of code are required to run after newEntity.Instance has been initialized. ]]
	
	if attributes["Owner"] then

		if owner then
			setNetworkOwner(owner, newEntity.Instance, true)
		else
			setNetworkOwner(Players:FindFirstChild(attributes["Owner"]), newEntity.Instance, true)

			local function onPlayerAdded(player)
				if player.Name == attributes["Owner"] then
					setNetworkOwner(player, newEntity.Instance, true)
					
					owner = player
				end
			end
			
			for _, player in pairs(Players:GetPlayers()) do
				onPlayerAdded(player)
			end
			
			table.insert(newEntity.Connections, Players.PlayerAdded:Connect(onPlayerAdded))
			
		end
	end
	
	if attributes["Health"] then
		newEntity.Instance:GetAttributeChangedSignal("Health"):Connect(function()
			local health = newEntity.Instance:GetAttribute("Health")
			if health and health <= 0 then
				newEntity:Destroy()
			end
		end)
	end
	
	newEntity:PivotTo(cframe)
	

	if attributes["ApplianceType"] then
		local module = game:GetService("ServerScriptService").Libraries:FindFirstChild(attributes["ApplianceType"])

		if module then
			warn(attributes["Owner"])
			require(module).new(newEntity.Instance, attributes["Level"], attributes["Owner"])
		end
	end
	
	newEntity.Instance.Name = instance.Name
	

	CurrEntities[newEntity.Instance] = newEntity
	--[[ Object has been successfully created. ]]
	
	return newEntity
end


--[[
	FUNCTION NAME: InitAttributes 
	PARAMETERS: Models/Parts/Tools, string, table
	DESCRIPTION: Initializes attributes for Entity to combine old and new attributes.
	Also used to apply attributes onto object.
	
	Saved data takes priority over default values.
	
	RETURNS: table containing finalized attributes
]]
function Entity:InitAttributes(instance, owner, currAttributes)
	local attributes = instance:GetAttributes()

	if attributes then
		for key, val in pairs(currAttributes) do
			attributes[key] = val
		end
	end

	if not attributes["Owner"] then
		if owner then
			attributes["Owner"] = owner.Name
		end
	end

	if attributes["Text"] then

		local success, response = pcall(function()
			local TextService = game:GetService("TextService")
			if owner and typeof(attributes["Owner"]) == "string" then
				local text = TextService:FilterStringAsync(attributes["Text"], owner.UserId):GetNonChatStringForBroadcastAsync()
				attributes["Text"] = text
			end

			instance.FrontGui.TextLabel.Text = attributes["Text"]
			instance.BackGui.TextLabel.Text = attributes["Text"]
		end)

		if not success then
			warn(response)
		end

	end
	
	return attributes
end


--[[
	FUNCTION NAME: PivotTo 
	PARAMETERS: cframe/model/table
	DESCRIPTION: Manages unique behavior amongst various data types related to pivoting.
	In the end, object should be placed at a particular cframe.
	
	RETURNS: N/A
]]
function Entity:PivotTo(cframe)
	if cframe then

		--*****if it's a model then why need to check if it's a cframe?*****
		if typeof(cframe) ~= "CFrame" and cframe:IsA("Model") then -- if uses character as a reference..?

			local tCFrame
			local offset = Vector3.new(0, self.Instance.PrimaryPart.Size.Y / 2, 0)

			local raycastParams = RaycastParams.new()
			raycastParams.FilterType = Enum.RaycastFilterType.Blacklist
			raycastParams.FilterDescendantsInstances = {cframe}

			local direction = cframe.Head.CFrame.LookVector * 10
			local raycastResult = workspace:Raycast(cframe.Head.Position, direction, raycastParams)

			if raycastResult then
				tCFrame = CFrame.new(raycastResult.Position + offset)
			else
				tCFrame = CFrame.new(cframe.Head.Position + (direction) + offset)
			end

			self.Instance:PivotTo(tCFrame)

		elseif typeof(cframe) == "CFrame" then

			self.Instance:PivotTo(cframe)

		elseif typeof(cframe) == "table" then

			for instanceName, cframe in pairs(cframe) do
				local success, response = pcall(function()
					self[instanceName].CFrame = cframe
				end)

				if not success then
					warn(response)
				end
			end

		end

	end
end


--[[
	FUNCTION NAME: Destroy 
	PARAMETERS: N/A
	DESCRIPTION: Clears object from memory. Destructor.
	
	RETURNS: N/A
]]
function Entity:Destroy()
	for _, conn in pairs(self.Connections) do
		conn:Disconnect()
	end
	
	local entity = self.Instance
	CurrEntities[self.Instance] = nil
	entity:Destroy()
end


--[[
	FUNCTION NAME: InitServer 
	PARAMETERS: N/A
	DESCRIPTION: Handles server-side behavior of loading all previously saved entities into the visible workspace.
	
	RETURNS: N/A
]]
if not Entity.InitServer then
	Entity.InitServer = true
	local savedEntities = dataStore:GetAsync(savedEntityKey)

	if savedEntities then
		print("Loading saved entities...")
		local entities = game:GetService("ServerStorage").Entities:GetDescendants()

		for key, value in pairs(savedEntities) do
			task.spawn(function()
				local success, response = pcall(function()
					-- Linear probing for unique keys within hashmap.
					local key_split =  string.split(key, "_")

					local matchedEntity

					local index = 0

					repeat
						
						index += 1
						matchedEntity = entities[index]

					until matchedEntity.Name == key_split[1] or index > #entities
					
					
					local tValue = value
					
					
					-- Initializing unique cframe behaviors...
					local cframe = tValue["_CF"]
					tValue["_CF"] = nil
					
					local cframeTbl = {}
					for key, attributeValue in pairs(value) do
						if key:sub(1, 3) == "_CF" and key ~= "_CF" then
							cframeTbl[key:sub(3, #key)] = stringtocf(cframe)
						end
					end
					
					local attributes = tValue
					if matchedEntity and index <= #entities then
						if #cframeTbl > 0 then
							local newEntity = Entity.new(matchedEntity:Clone(), cframeTbl, attributes)
						else
							local newEntity = Entity.new(matchedEntity:Clone(), stringtocf(cframe), attributes)
						end

					else
						error(key_split[1] .. " was not found...")
					end
				end)
				if not success then
					warn(response)
				end
			end)
		end
	end
end

return Entity
