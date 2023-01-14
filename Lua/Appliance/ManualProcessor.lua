-- External Dependencies.
local Appliance = require(script.Parent)

local Processor = {}
Processor.__index = Processor
setmetatable(Processor, Appliance)


--[[ PRIVATE LOCAL VARIABLES]]
local PROCESSING_TIME = 10
local DEBUG_PROCESSING_COLORS = {"Really red", "Persimmon", "Salmon", "Deep orange",  "Gold", "Bright yellow", "Medium green", "Olivine", "Bright green", "Forest green"}

--[[
	FUNCTION NAME: Constructor
	PARAMETERS: Instance
	DESCRIPTION: Inherits default values and methods from Appliance Object. 
	Requires player to manually increment process of ingredient and locks character in place.
	
	
	RETURNS: Processor Object
]]
function Processor.new(instance)
	local newProcessor = Appliance.new(instance)
	setmetatable(newProcessor, Processor)

	newProcessor.RequiredIngredient = instance:GetAttribute("Ingredient")
	newProcessor.Product = instance:GetAttribute("Product")
	
	newProcessor.User = nil -- whoever is using the machine currently
	
	return newProcessor
end


--[[
	FUNCTION NAME: Activate
	PARAMETERS: N/A
	DESCRIPTION: Abstract action of Activate. Manually increments progress of item by 1.
	
	
	RETURNS: N/A												
]]
function Processor:Activate()
	print("Activated ManualProcessor")
	if self.Item then
		
		-- if there hasn't been any progress on the item yet, let's make sure it's added now...
		if not self.Item.Instance:GetAttribute("Progress") then
			self.Item.Instance:SetAttribute("Progress", 1)
		end
		
		-- if required ingredient and ingredient don't match... stop function...
		if self.Item.Instance:GetAttribute("EntityType") ~= self.RequiredIngredient then
			warn("Wrong Item!!! | " .. self.Item.Instance:GetAttribute(("EntityType")))
			self.Completed = true
			
			return	
		end
		
		
		if self.Item.Instance:GetAttribute("Progress") >= PROCESSING_TIME then
			self.Completed = true
		else
			self.Completed = false
		end
	else

		return
	end
	
	

end

--[[
	FUNCTION NAME: StartProcess
	PARAMETERS: Player
	DESCRIPTION: Assigns player to machine and provides visual feedback that player is using appliance.
	Anchors player's character into a particular positioin.
	
	RETURNS: N/A												
]]
function Processor:StartProcess(player)
	local currItem = self.Item
	

	
	if self.Item and self.Item == currItem and not self.User then
		
		
		print(player.Name, "is interacting w/ ManualProcessor")
		self.User = player
		self.Item.CanGrab = false
		
		

		
		local applianceCFrame = self.Instance.PrimaryPart.CFrame
		local appliancePos = applianceCFrame.Position

		local _, playerSize = player.Character:GetBoundingBox()

		local playerStandingLoc = appliancePos - Vector3.new(0, self.Instance.PrimaryPart.Size.Y / 2 - playerSize.Y / 2, 0)
		local playerFacingPoint = Vector3.new(appliancePos.X, appliancePos.Y - self.Instance.PrimaryPart.Size.Y / 2 + playerSize.Y / 2, appliancePos.Z)
		player.Character:PivotTo(CFrame.new(playerStandingLoc + applianceCFrame.LookVector * Vector3.new(5, 5, 5), playerFacingPoint)) 
		
		player.Character.PrimaryPart.Anchored = true
		
		
		-- starts connection for process event
		self:Process()

	end
end

--[[
	FUNCTION NAME: Process
	PARAMETERS: Player
	DESCRIPTION: Contains listener for when player wishes to increment progress of ingredient.

	
	
	RETURNS: N/A												
]]
function Processor:Process(player)
	local InteractEvent = game:GetService("ReplicatedStorage").Remotes.InteractEvent
	-- clean up this event?
	self.Connection = InteractEvent.OnServerEvent:Connect(function(player) 
		print("Attempting connection!")
		-- this connection is only for user...
		if player ~= self.User then
			return
		end
		-- controls and shows progress to user...
		local progress = self.Item.Instance:GetAttribute("Progress")
		self.Item.Instance:SetAttribute("Progress", progress + 1)

		for _, part in pairs(self.Item.Instance:GetChildren()) do
			if part:IsA("BasePart") then
				part.Size += Vector3.new(0, -.5, 0)
				part.BrickColor = BrickColor.new(DEBUG_PROCESSING_COLORS[progress])
			end
		end

		self.Item:PivotTo(self.Instance.PrimaryPart.CFrame + Vector3.new(0, self.Instance.PrimaryPart.Size.Y / 2 + self.Item.PrimaryPart.Size.Y / 2, 0))
		print("Progress: " .. progress .. " / " .. PROCESSING_TIME)
		self.ItemGui:SetProgressBar(progress/PROCESSING_TIME)


		if progress == PROCESSING_TIME then
			self:EndProcess(player)
		end
	end)
end


--[[
	FUNCTION NAME: EndProcess
	PARAMETERS: Player
	DESCRIPTION: Ends listening event for player on appliance. Also unanchors player after deconstucting object to default values.
	
	
	RETURNS: N/A												
]]
function Processor:EndProcess(player)
	
	self.ItemGui:SetProgressBar(0)

	self.Connection:Disconnect()
	self.User = nil
	self.Item.CanGrab = true
	self.Completed = true
	
	player.Character.PrimaryPart.Anchored = false
end
return Processor
