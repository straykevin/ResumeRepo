local UserInputService = game:GetService("UserInputService")
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Promise = require(ReplicatedStorage.Libraries.Promises)
local Maid = require(ReplicatedStorage.Libraries.Maids)
local Spring = require(ReplicatedStorage.Libraries.Springs)

local GunEvent = ReplicatedStorage.CoreRemotes.GunEvent
local player = Players.LocalPlayer
local Mouse = player:GetMouse()




local Guns = {}
Guns.__index = Guns

function Guns.new(tool)
	local self = setmetatable({}, Guns)
	
	
	-- Private Variables
	self._Instance = tool
	self._Config = tool:WaitForChild("Configuration")
	self._Mag = self._Config.Ammo.Magazines
	self._AmmoGui = Players.LocalPlayer.PlayerGui.CoreGui.AmmoGui
	self._AmmoLabel = self._AmmoGui.AmmoLabel
	
	self._Animations = {
		Hold = {Animation = tool:WaitForChild("Hold"), FadeTime = nil, Weight = nil, Speed = 1, Duration = 2},
		Fire = {Animation = tool:WaitForChild("Fire"), FadeTime = 0.25, Weight = nil, Speed = 0.5, Duration = 0.5},
		Reload = {Animation = tool:WaitForChild("Reload"), FadeTime = nil, Weight = nil, Speed = 1, Duration = 4},
	}
	
	-- Private Variables
	self.Reloading = false
	self.Toggle = false
	
	
	self.Maid = Maid.new()
	self.AnimationTracks = {}

	
	--[[ Handles Touch Input for shooting and reloading...]]-- 
	self.Maid:GiveTask(self._AmmoGui.InteractGui.MouseButton1Down:Connect(function()
		self:ToggleFire()
	end))
	
	self.Maid:GiveTask(self._AmmoGui.InteractGui.MouseButton1Up:Connect(function()
		if self._Mag.Value > 0 and not self.Reloading then
			self.Toggle = false
		end
	end))
	
	self.Maid:GiveTask(self._AmmoGui.MouseButton1Down:Connect(function()
		self:Reload()
	end))
	
	
	--[[ Handles Context-Based Actions for MouseButton.]]--
	self.Maid:GiveTask(UserInputService.InputBegan:Connect(function(input, gameProcessedEvent)
		if not tool then return end



		if gameProcessedEvent then return end

		if input.UserInputType == Enum.UserInputType.MouseButton1 then
			self:ToggleFire()
		elseif input.UserInputType == Enum.UserInputType.Keyboard then
			if input.KeyCode == Enum.KeyCode.R then
				warn("beep boop")
				self:Reload()
			end
		end
	end))
	
	self.Maid:GiveTask(UserInputService.InputEnded:Connect(function(input, gameProcessedEvent)
		if gameProcessedEvent then return end

		if input.UserInputType == Enum.UserInputType.MouseButton1 then
			if self._Mag.Value > 0 and not self.Reloading then
				self.Toggle = false
			end
		end
	end))
	
	return self
end

function Guns:SetAnimation(mode, value)
	if mode == "PlayAnimation" and value then
		for i, v in pairs(self.AnimationTracks) do
			if v.Animation == value.Animation then
				v.AnimationTrack:Stop()
				table.remove(self.AnimationTracks, i)
			end
		end
		local AnimationTrack = player.Character.Humanoid.Animator:LoadAnimation(value.Animation)
		table.insert(self.AnimationTracks, {Animation = value.Animation, AnimationTrack = AnimationTrack})
		AnimationTrack:Play(value.FadeTime, value.Weight, value.Speed)
	elseif mode == "StopAnimation" and value then
		for i, v in pairs(self.AnimationTracks) do
			if v.Animation == value.Animation then
				v.AnimationTrack:Stop(value.FadeTime)
				table.remove(self.AnimationTracks, i)
			end
		end
	end
end

function Guns:Reload()
	if not self.Reloading then
		self.Reloading = true
		
		task.spawn(function()
			local Animation = self._Animations.Reload
			self:SetAnimation("PlayAnimation", Animation)
			task.wait(Animation.Duration)
			self:SetAnimation("StopAnimation", Animation)
		end)
		
		GunEvent:FireServer("Reload")
	
		task.wait(self._Config.ReloadTime.Value)
		
		self._Mag.Value = self._Mag.MaxValue
		self.Reloading = false

		self._AmmoLabel.Visible = true
		self._AmmoLabel.Text = self._Mag.Value .. " / " .. self._Mag.MaxValue

	else
		print("Player is already reloading!!!")
	end
end

function Guns:Fire()

	self._Mag.Value -= 1

	local targetDir = (workspace.CurrentCamera.CFrame.Position - workspace.CurrentCamera.Focus.Position).Unit

	local raycastParams = RaycastParams.new()
	raycastParams.FilterType = Enum.RaycastFilterType.Blacklist
	raycastParams.FilterDescendantsInstances = {player.Character}
	raycastParams.RespectCanCollide = true

	local raycastResult = workspace:Raycast(workspace.CurrentCamera.CFrame.Position, -targetDir * 1000, raycastParams)
	local endPos = workspace.CurrentCamera.CFrame.Position + targetDir * 1000


	GunEvent:FireServer("Fire", (raycastResult and raycastResult.Position) or endPos, self._Instance:GetAttributes()) -- Mouse.Hit.Position
	self._AmmoLabel.Text = self._Mag.Value .. " / " .. self._Mag.MaxValue
end

function Guns:ToggleFire()
	if self._Mag.Value > 0 and not self.Reloading then

		task.spawn(function()
			self.Toggle = true

			if self._Config.Automatic.Value then
				while self.Toggle and not self.Reloading and self._Mag.Value > 0 do
					self:Fire()							
					task.wait(.09)
				end
			else
				self:Fire()	
			end

			self._AmmoLabel.Visible = true
			self._AmmoLabel.Text = self._Mag.Value .. " / " .. self._Mag.MaxValue

			self.Toggle = false
		end)

	else
		GunEvent:FireServer("No Ammo")
	end
end

function Guns:Destroy()
	self.Maid:DoCleaning()
end

return Guns
