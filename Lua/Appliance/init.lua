local Appliance = {}
Appliance.__index = Appliance


--[[
	FUNCTION NAME: Abstract Constructor
	PARAMETERS: Instance
	DESCRIPTION: Purely virtual functions. Will provided inherited functions to children of this class.
	
	
	RETURNS: Appliance Object
]]
function Appliance.new(instance)
	local newAppliance = {}
	setmetatable(newAppliance, Appliance)

	newAppliance.Name = instance.Name
	newAppliance.Instance = instance
	newAppliance.InUse = false
	newAppliance.Item = nil
	newAppliance.Completed = false
	newAppliance.ItemGui = require(script.Parent.ItemGui).new(instance)

	return newAppliance
end


--[[
	FUNCTION NAME: EquipItem
	PARAMETERS: `Instance`
	DESCRIPTION: Saves Item under Appliance to be used as reference for child functions.
	
	
	RETURNS: N/A
]]
function Appliance:EquipItem(item)
	
	if not self.InUse then
		self.Item = item
		self.InUse = true
		
		item:Anchor(true)
		item:PivotTo(self.Instance.PrimaryPart.CFrame + Vector3.new(0, self.Instance.PrimaryPart.Size.Y / 2 + item.PrimaryPart.Size.Y / 2, 0))
		item.Parent = self
		item.Active = true
		
		if self.Activate then
			self:Activate()
		else
			warn(self)
		end
	end
end


--[[
	FUNCTION NAME: UnequipItem
	PARAMETERS: `N/A`
	DESCRIPTION: Removes item from appliances.
	
	
	RETURNS: N/A
]]
function Appliance:UnequipItem()
	self.Item = nil
	self.InUse = false
	
	
end

return Appliance
