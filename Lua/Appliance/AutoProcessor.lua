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
	
	
	RETURNS: Processor Object
]]
function Processor.new(instance)
	local newProcessor = Appliance.new(instance)
	setmetatable(newProcessor, Processor)

	newProcessor.RequiredIngredient = instance:GetAttribute("Ingredient")
	newProcessor.Product = instance:GetAttribute("Product")
	
	return newProcessor
end


--[[
	FUNCTION NAME: Activate
	PARAMETERS: N/A
	DESCRIPTION: Abstract action of Activate. Automatically increments progress of item by 1. Does not require player to maintain position.
	
	
	RETURNS: N/A												
]]
function Processor:Activate()
	local currItem = self.Item -- let's watch out incase of item changes...
	
	-- if required ingredient and ingredient don't match... stop function...
	if self.Item.Instance:GetAttribute("EntityType") ~= self.RequiredIngredient then
		return	
	end
	
	-- if there hasn't been any progress on the item yet, let's make sure it's added now...
	if not self.Item.Instance:GetAttribute("Progress") or self.Item.Instance:GetAttribute("Progress") < 1 then
		self.Item.Instance:SetAttribute("Progress", 1)
	end
	
	self.ItemGui:SetProgressBar(0, true)
	while self.Item == currItem and self.Item.Instance:GetAttribute("Progress") <= PROCESSING_TIME do
		local progress = self.Item.Instance:GetAttribute("Progress")
		task.wait(1)
		if self.Item == currItem then
			self.Item.Instance:SetAttribute("Progress", progress + 1)
			
			for _, part in pairs(self.Item.Instance:GetChildren()) do
				if part:IsA("BasePart") then
					part.Size += Vector3.new(0, .5, 0)
					part.BrickColor = BrickColor.new(DEBUG_PROCESSING_COLORS[progress])
				end
			end
			
			self.Item:PivotTo(self.Instance.PrimaryPart.CFrame + Vector3.new(0, self.Instance.PrimaryPart.Size.Y / 2 + self.Item.PrimaryPart.Size.Y / 2, 0))
			
			self.ItemGui:SetProgressBar(progress/PROCESSING_TIME)
			print("Progress: " .. progress .. " / " .. PROCESSING_TIME)
		end
	end
	
	self.ItemGui:SetProgressBar(0)

	-- If the item isn't the same at this point, its gone!
	if self.Item ~= currItem then
		return 
	end

	-- assume at this point it's max level...
	currItem.Instance:SetAttribute("EntityType", self.Product)
	currItem.Active = false
	currItem.Instance:SetAttribute("Progress", 1)
end

return Processor
